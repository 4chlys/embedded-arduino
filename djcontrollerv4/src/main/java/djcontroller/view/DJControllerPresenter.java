package main.java.djcontroller.view;

import main.java.djcontroller.model.ArduinoModel;
import main.java.djcontroller.model.PlaylistModel;
import main.java.djcontroller.view.DJControllerView;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Presenter Class - handles the communication between the view and the model
 */
public class DJControllerPresenter {
    private final PlaylistModel playlistModel;
    private final ArduinoModel arduinoModel;

    private final DJControllerView view;

    private final AtomicBoolean seekUpdateInProgress = new AtomicBoolean(false);
    private final AtomicBoolean volumeUpdateInProgress = new AtomicBoolean(false);

    /**
     * Constructor
     *
     * @param view          The view implementation
     * @param playlistModel The playlist model
     * @param arduinoModel  The Arduino communication model
     */
    public DJControllerPresenter(DJControllerView view, PlaylistModel playlistModel, ArduinoModel arduinoModel) {
        this.view = view;
        this.playlistModel = playlistModel;
        this.arduinoModel = arduinoModel;

        initialize();
    }

    /**
     * Initialize the presenter
     */
    private void initialize() {
        view.initialize();
        setupModelCallbacks();
        addEventHandlers();
        updateView();
    }

    /**
     * Set up callbacks from models to presenter
     */
    private void setupModelCallbacks() {
        playlistModel.setPlayStateChangeCallback(this::handlePlayStateChange);
        playlistModel.setTrackChangeCallback(this::handleTrackChange);
        playlistModel.setPositionChangeCallback(this::handlePositionChange);
        playlistModel.setVolumeChangeCallback(this::handleVolumeChange);
        playlistModel.setBeatDetectedCallback(this::handleBeatDetected);

        arduinoModel.setStatusChangeCallback(this::handleArduinoStatusChange);

        arduinoModel.setPlayHandler(playlistModel::play);

        arduinoModel.setPauseHandler(playlistModel::pause);

        arduinoModel.setNextTrackHandler(() -> {
            boolean wasPlaying = playlistModel.isPlaying();
            int oldTrack = playlistModel.getCurrentTrackIndex();

            playlistModel.nextTrack();

            if (playlistModel.getCurrentTrackIndex() != oldTrack) {
                javafx.application.Platform.runLater(() -> {
                    arduinoModel.sendTrackInfo(
                            playlistModel.getCurrentTrackIndex() + 1,
                            playlistModel.getTrackCount()
                    );
                });
            }
        });

        arduinoModel.setPrevTrackHandler(() -> {
            boolean wasPlaying = playlistModel.isPlaying();
            int oldTrack = playlistModel.getCurrentTrackIndex();

            playlistModel.previousTrack();

            if (playlistModel.getCurrentTrackIndex() != oldTrack) {
                javafx.application.Platform.runLater(() -> {
                    arduinoModel.sendTrackInfo(
                            playlistModel.getCurrentTrackIndex() + 1,
                            playlistModel.getTrackCount()
                    );
                });
            }
        });

        arduinoModel.setSeekHandler(playlistModel::seekByRelativeSeconds);

        arduinoModel.setStatusRequestHandler(() -> {
            arduinoModel.sendTrackInfo(
                    playlistModel.getCurrentTrackIndex() + 1,
                    playlistModel.getTrackCount()
            );
            arduinoModel.sendPlayStatus(playlistModel.isPlaying());
        });
    }

    /**
     * Add event handlers to view components
     * This is where we connect user actions to model operations
     */
    private void addEventHandlers() {
        view.getConnectButton().setOnAction(e -> {
            if (view.getConnectButton().isSelected()) {
                String selectedPort = view.getPortList().getValue();
                if (selectedPort != null && !selectedPort.isEmpty()) {
                    onConnectRequest(selectedPort);
                } else {
                    view.getConnectButton().setSelected(false);
                    view.showError("No port selected");
                }
            } else {
                onDisconnectRequest();
            }
        });

        view.getRefreshButton().setOnAction(e -> onRefreshPortsRequest());

        view.getAddButton().setOnAction(e -> {
            List<File> files = view.showFileChooser();
            if (files != null && !files.isEmpty()) {
                onFilesSelected(files);
            }
        });

        view.getRemoveButton().setOnAction(e -> {
            int selectedIndex = view.getPlaylistView().getSelectionModel().getSelectedIndex();
            if (selectedIndex >= 0) {
                onRemoveTrackRequest(selectedIndex);
            }
        });

        view.getClearButton().setOnAction(e -> onClearPlaylistRequest());

        view.getPlaylistView().getSelectionModel().selectedIndexProperty().addListener(
                (obs, oldValue, newValue) -> {
                    if (newValue != null && newValue.intValue() >= 0) {
                        onTrackSelected(newValue.intValue());
                    }
                }
        );

        view.getPlayButton().setOnAction(e -> {
            if (view.getPlayButton().getText().equals("▶")) {
                onPlayRequest();
            } else {
                onPauseRequest();
            }
        });

        view.getPrevButton().setOnAction(e -> onPrevTrackRequest());

        view.getNextButton().setOnAction(e -> onNextTrackRequest());

        view.getSeekSlider().valueProperty().addListener((obs, oldVal, newVal) -> {
            if (view.getSeekSlider().isValueChanging() && !seekUpdateInProgress.get()) {
                onSeekRequest(newVal.intValue());
            }
        });

        view.getVolumeSlider().valueProperty().addListener((obs, oldVal, newVal) -> {
            if (!volumeUpdateInProgress.get()) {
                volumeUpdateInProgress.set(true);
                try {
                    onVolumeChange(newVal.intValue());
                } finally {
                    volumeUpdateInProgress.set(false);
                }
            }
        });

        view.getPrimaryStage().setOnCloseRequest(e -> {
            onDisconnectRequest();
            playlistModel.stop();
            arduinoModel.sendPlayStatus(false);
        });
    }

    /**
     * Update the view with current model state
     * This method updates all aspects of the view to match the model state
     */
    private void updateView() {
        view.updateConnectionStatus(arduinoModel.isConnected());
        view.updatePortList(arduinoModel.getAvailablePorts());

        updatePlaylistView();
        updateTrackInfoView();

        view.updatePlaybackStatus(playlistModel.isPlaying());
        view.updateVolume(playlistModel.getVolume());

        updateTimeDisplay();
    }

    /**
     * Update the playlist view
     */
    private void updatePlaylistView() {
        List<String> trackNames = new ArrayList<>();
        for (int i = 0; i < playlistModel.getTrackCount(); i++) {
            trackNames.add(playlistModel.getTrackName(i));
        }
        view.updatePlaylist(trackNames, playlistModel.getCurrentTrackIndex());
    }

    /**
     * Update the track info view
     */
    private void updateTrackInfoView() {
        if (playlistModel.getTrackCount() > 0) {
            int currentIndex = playlistModel.getCurrentTrackIndex();
            String trackName = playlistModel.getTrackName(currentIndex);
            view.updateTrackInfo(trackName, currentIndex + 1, playlistModel.getTrackCount());
        } else {
            view.updateTrackInfo("No track loaded", 0, 0);
        }
    }

    /**
     * Update the time display
     */
    private void updateTimeDisplay() {
        if (playlistModel.getTrackCount() > 0) {
            int currentPos = playlistModel.getCurrentPositionInSeconds();
            int totalDuration = playlistModel.getTrackDuration(playlistModel.getCurrentTrackIndex());

            int currentMinutes = currentPos / 60;
            int currentSeconds = currentPos % 60;
            int totalMinutes = totalDuration / 60;
            int totalSeconds = totalDuration % 60;

            view.updateTimeDisplay(currentMinutes, currentSeconds, totalMinutes, totalSeconds);

            seekUpdateInProgress.set(true);
            try {
                int positionPercentage = (totalDuration > 0) ?
                        (int) (currentPos * 100.0 / totalDuration) : 0;
                view.updateSeekPosition(positionPercentage);
            } finally {
                seekUpdateInProgress.set(false);
            }
        } else {
            view.updateTimeDisplay(0, 0, 0, 0);

            seekUpdateInProgress.set(true);
            try {
                view.updateSeekPosition(0);
            } finally {
                seekUpdateInProgress.set(false);
            }
        }
    }

    /**
     * Send full status information to Arduino using single-byte protocol
     */
    private void sendFullStatusToArduino() {
        if (playlistModel.getTrackCount() > 0) {
            arduinoModel.sendFullStatusUpdate(
                    playlistModel.isPlaying(),
                    playlistModel.getCurrentTrackIndex() + 1,
                    playlistModel.getTrackCount()
            );
        } else {
            arduinoModel.resetTrackCounters();
        }
    }

    /**
     * Handle play state change from model
     *
     * @param isPlaying New play state
     */
    private void handlePlayStateChange(boolean isPlaying) {
        view.updatePlaybackStatus(isPlaying);

        arduinoModel.sendPlayStatus(isPlaying);
    }

    /**
     * Handle track change from model - updated for single-byte protocol
     *
     * @param newTrackIndex New track index
     */
    private void handleTrackChange(int newTrackIndex) {
        updateTrackInfoView();
    }

    /**
     * Handle position change from model
     *
     * @param newPosition New position in seconds
     */
    private void handlePositionChange(int newPosition) {
        updateTimeDisplay();
    }

    /**
     * Handle volume change from model
     *
     * @param newVolume New volume level
     */
    private void handleVolumeChange(int newVolume) {
        volumeUpdateInProgress.set(true);
        try {
            view.updateVolume(newVolume);
        } finally {
            volumeUpdateInProgress.set(false);
        }
    }

    /**
     * Handle beat detected from model
     */
    private void handleBeatDetected() {
        arduinoModel.sendBeatDetected();
    }

    /**
     * Handle Arduino status change
     *
     * @param status New status message
     */
    private void handleArduinoStatusChange(String status) {
        view.showStatus(status);
    }

    /**
     * Handle connect request
     *
     * @param portName Port name to connect to
     */
    private void onConnectRequest(String portName) {
        boolean success = arduinoModel.connect(portName);
        if (success) {
            view.updateConnectionStatus(true);

            sendFullStatusToArduino();
        } else {
            view.updateConnectionStatus(false);
        }
    }

    /**
     * Handle disconnect request
     */
    private void onDisconnectRequest() {
        if (playlistModel.isPlaying()) {
            playlistModel.pause();
            arduinoModel.sendPlayStatus(false);
        }

        arduinoModel.disconnect();
        view.updateConnectionStatus(false);
    }

    /**
     * Handle refresh ports request
     */
    private void onRefreshPortsRequest() {
        view.updatePortList(arduinoModel.getAvailablePorts());
    }

    /**
     * Handle remove track request
     *
     * @param index Index of track to remove
     */
    private void onRemoveTrackRequest(int index) {
        playlistModel.removeTrack(index);
        updatePlaylistView();

        if (playlistModel.getTrackCount() > 0) {
            sendFullStatusToArduino();
        } else {
            arduinoModel.sendPlayStatus(false);
        }
    }

    /**
     * Handle clear playlist request - updated for single-byte protocol
     */
    private void onClearPlaylistRequest() {
        playlistModel.clearPlaylist();
        updatePlaylistView();
        updateTrackInfoView();
        updateTimeDisplay();

        arduinoModel.resetTrackCounters();
        arduinoModel.sendPlayStatus(false);
    }

    /**
     * Handle track selection
     *
     * @param index Index of selected track
     */
    private void onTrackSelected(int index) {
        playlistModel.setCurrentTrackIndex(index);
    }

    /**
     * Handle play request
     */
    private void onPlayRequest() {
        playlistModel.play();
        arduinoModel.sendPlayStatus(true);
    }

    /**
     * Handle pause request
     */
    private void onPauseRequest() {
        playlistModel.pause();
        arduinoModel.sendPlayStatus(false);
    }

    /**
     * Handle next track request - updated for single-byte protocol
     */
    private void onNextTrackRequest() {
        int oldIndex = playlistModel.getCurrentTrackIndex();
        playlistModel.nextTrack();
        int newIndex = playlistModel.getCurrentTrackIndex();

        if (newIndex != oldIndex) {
            arduinoModel.sendTrackChange(true, playlistModel.isPlaying());
        }
    }

    /**
     * Handle previous track request - updated for single-byte protocol
     */
    private void onPrevTrackRequest() {
        int oldIndex = playlistModel.getCurrentTrackIndex();
        playlistModel.previousTrack();
        int newIndex = playlistModel.getCurrentTrackIndex();

        if (newIndex != oldIndex) {
            arduinoModel.sendTrackChange(false, playlistModel.isPlaying());
        }
    }

    /**
     * Handle seek request
     *
     * @param position Position as percentage (0-100)
     */
    private void onSeekRequest(int position) {
        playlistModel.seekByPercentage(position);
    }

    /**
     * Handle volume change
     *
     * @param volume New volume level (0-100)
     */
    private void onVolumeChange(int volume) {
        playlistModel.setVolume(volume);
    }

    /**
     * Handle files selected for playlist - updated for single-byte protocol
     *
     * @param files List of selected files
     */
    private void onFilesSelected(List<File> files) {
        boolean wasEmpty = playlistModel.getTrackCount() == 0;

        for (File file : files) {
            playlistModel.addTrack(file);
        }
        updatePlaylistView();

        if (wasEmpty && playlistModel.getTrackCount() > 0) {
            playlistModel.setCurrentTrackIndex(0);
            sendFullStatusToArduino();
        } else if (playlistModel.getTrackCount() > 0) {
            arduinoModel.sendTrackInfo(
                    playlistModel.getCurrentTrackIndex() + 1,
                    playlistModel.getTrackCount()
            );
        }
    }
}