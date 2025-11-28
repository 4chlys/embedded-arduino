package main.java.djcontroller.view;

import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.FileChooser;
import javafx.stage.Stage;

import java.io.File;
import java.net.URL;
import java.util.List;

/**
 * DJControllerView - handles the view implementation for the DJController application
 */
public class DJControllerView {
    private BorderPane root;

    private ComboBox<String> portList;
    private ToggleButton connectButton;
    private Button refreshButton;
    private Label statusLabel;

    private ListView<String> playlistView;
    private Label trackInfoLabel;
    private Button addButton;
    private Button removeButton;
    private Button clearButton;

    private Button prevButton;
    private Button playButton;
    private Button nextButton;
    private Slider seekSlider;
    private Label timeLabel;
    private Slider volumeSlider;
    private ProgressBar volumeBar;

    private final Stage primaryStage;

    /**
     * Constructor
     * @param primaryStage The primary stage for this view
     */
    public DJControllerView(Stage primaryStage) {
        this.primaryStage = primaryStage;
    }

    /**
     * Initialize the view
     */
    public void initialize() {
        primaryStage.setTitle("DJ Controller");

        root = new BorderPane();

        initialiseNodes();
        layoutNodes();

        Scene scene = new Scene(root, 800, 600);
        try {
            String cssPath = "/styles/DJController.css";
            URL cssResource = getClass().getResource(cssPath);

            if (cssResource == null) {
                cssResource = getClass().getResource("/main/resources" + cssPath);
            }

            if (cssResource == null) {
                cssResource = getClass().getClassLoader().getResource("styles/DJController.css");
            }

            if (cssResource == null) {
                cssResource = getClass().getClassLoader().getResource("main/resources/styles/DJController.css");
            }
            if (cssResource != null) {
                scene.getStylesheets().add(cssResource.toExternalForm());
                System.out.println("CSS loaded successfully from: " + cssResource.toExternalForm());
            } else {
                File cssFile = new File("src/main/resources/styles/DJController.css");
                if (cssFile.exists()) {
                    scene.getStylesheets().add(cssFile.toURI().toURL().toExternalForm());
                    System.out.println("CSS loaded from file system: " + cssFile.getAbsolutePath());
                } else {
                    System.err.println("Could not find CSS file. Paths tried:");
                    System.err.println("- /styles/DJController.css");
                    System.err.println("- /main/resources/styles/DJController.css");
                    System.err.println("- src/main/resources/styles/DJController.css");
                }
            }
        } catch (Exception e) {
            System.err.println("Error loading CSS: " + e.getMessage());
            e.printStackTrace();
        }

        primaryStage.setScene(scene);
        primaryStage.show();
    }

    /**
     * Initialize all UI nodes/components
     */
    private void initialiseNodes() {
        portList = new ComboBox<>();
        portList.setPromptText("Select Port");

        connectButton = new ToggleButton("Connect");
        refreshButton = new Button("Refresh");
        statusLabel = new Label("Not connected");

        playlistView = new ListView<>();
        trackInfoLabel = new Label("No track loaded");

        addButton = new Button("Add Tracks");
        removeButton = new Button("Remove");
        clearButton = new Button("Clear All");

        prevButton = new Button("⏮");
        playButton = new Button("▶");
        nextButton = new Button("⏭");

        seekSlider = new Slider(0, 100, 0);
        seekSlider.setPrefWidth(400);

        timeLabel = new Label("00:00 / 00:00");

        volumeSlider = new Slider(0, 100, 50);
        volumeSlider.setPrefWidth(150);

        volumeBar = new ProgressBar(0.5);
        volumeBar.setPrefWidth(150);
        volumeBar.progressProperty().bind(volumeSlider.valueProperty().divide(100));
    }

    /**
     * Layout all UI nodes in appropriate containers
     */
    private void layoutNodes() {
        HBox connectionControls = new HBox(10);
        connectionControls.getStyleClass().add("connection-controls");
        connectionControls.setPadding(new Insets(10));
        connectionControls.setAlignment(Pos.CENTER_LEFT);
        connectionControls.getChildren().addAll(
                new Label("Port:"), portList,
                connectButton, refreshButton,
                new Separator(javafx.geometry.Orientation.VERTICAL),
                statusLabel
        );
        statusLabel.getStyleClass().add("status-label");
        root.setTop(connectionControls);

        VBox playlistControls = new VBox(10);
        playlistControls.getStyleClass().add("playlist-section");
        playlistControls.setPadding(new Insets(10));

        HBox trackInfoBox = new HBox(10);
        trackInfoBox.setAlignment(Pos.CENTER_LEFT);
        trackInfoBox.getChildren().add(trackInfoLabel);
        trackInfoLabel.getStyleClass().add("track-info-label");

        HBox playlistButtonsBox = new HBox(10);
        playlistButtonsBox.setAlignment(Pos.CENTER_LEFT);
        playlistButtonsBox.getChildren().addAll(addButton, removeButton, clearButton);

        addButton.getStyleClass().add("playlist-button");
        removeButton.getStyleClass().add("playlist-button");
        clearButton.getStyleClass().add("playlist-button");

        playlistControls.getChildren().addAll(
                trackInfoBox,
                new Label("Playlist:"),
                playlistView,
                playlistButtonsBox
        );
        root.setCenter(playlistControls);

        VBox playbackControls = new VBox(15);
        playbackControls.getStyleClass().add("playback-controls");
        playbackControls.setPadding(new Insets(15));

        HBox seekControls = new HBox(10);
        seekControls.getStyleClass().add("control-section");
        seekControls.setAlignment(Pos.CENTER);
        timeLabel.getStyleClass().add("time-label");
        seekSlider.getStyleClass().addAll("seek-slider");
        seekControls.getChildren().addAll(timeLabel, seekSlider);

        HBox playerControls = new HBox(20);
        playerControls.getStyleClass().add("control-section");
        playerControls.setAlignment(Pos.CENTER);

        prevButton.getStyleClass().addAll("transport-button");
        playButton.getStyleClass().addAll("transport-button", "play-button");
        nextButton.getStyleClass().addAll("transport-button");

        playerControls.getChildren().addAll(prevButton, playButton, nextButton);

        HBox volumeControls = new HBox(10);
        volumeControls.getStyleClass().add("control-section");
        volumeControls.setAlignment(Pos.CENTER);
        volumeSlider.getStyleClass().add("volume-slider");
        volumeControls.getChildren().addAll(new Label("Volume:"), volumeSlider, volumeBar);

        playbackControls.getChildren().addAll(seekControls, playerControls, volumeControls);
        root.setBottom(playbackControls);
    }

    /**
     * Show an error message
     * @param message The error message to display
     */
    public void showError(String message) {
        Platform.runLater(() -> {
            Alert alert = new Alert(Alert.AlertType.ERROR);
            alert.setTitle("Error");
            alert.setHeaderText(null);
            alert.setContentText(message);
            alert.showAndWait();
        });
    }

    /**
     * Update status message display
     * @param message Status message
     */
    public void showStatus(String message) {
        Platform.runLater(() -> statusLabel.setText(message));
    }

    /**
     * Update connection status in UI
     * @param isConnected Whether Arduino is connected
     */
    public void updateConnectionStatus(boolean isConnected) {
        Platform.runLater(() -> {
            connectButton.setSelected(isConnected);
            if (isConnected) {
                connectButton.setText("Disconnect");
            } else {
                connectButton.setText("Connect");
            }
        });
    }

    /**
     * Update the available ports list
     * @param ports List of available port names
     */
    public void updatePortList(List<String> ports) {
        Platform.runLater(() -> {
            portList.getItems().clear();
            portList.getItems().addAll(ports);
            if (!ports.isEmpty()) {
                portList.setValue(ports.get(0));
            }
        });
    }

    /**
     * Update the playlist display
     * @param trackNames List of track names
     * @param currentIndex Current track index
     */
    public void updatePlaylist(List<String> trackNames, int currentIndex) {
        Platform.runLater(() -> {
            playlistView.getItems().clear();
            playlistView.getItems().addAll(trackNames);
            if (currentIndex >= 0 && currentIndex < trackNames.size()) {
                playlistView.getSelectionModel().select(currentIndex);
            }
        });
    }

    /**
     * Update the track information display
     * @param trackName Name of the current track
     * @param currentTrack Current track number (1-based)
     * @param totalTracks Total number of tracks
     */
    public void updateTrackInfo(String trackName, int currentTrack, int totalTracks) {
        Platform.runLater(() -> {
            trackInfoLabel.setText(String.format("Track %d/%d: %s",
                    currentTrack, totalTracks, trackName));
        });
    }

    /**
     * Update the play/pause button state
     * @param isPlaying Whether a track is playing
     */
    public void updatePlaybackStatus(boolean isPlaying) {
        Platform.runLater(() -> {
            playButton.setText(isPlaying ? "⏸" : "▶");
        });
    }

    /**
     * Update the time display
     * @param currentMinutes Current minutes
     * @param currentSeconds Current seconds
     * @param totalMinutes Total minutes
     * @param totalSeconds Total seconds
     */
    public void updateTimeDisplay(int currentMinutes, int currentSeconds, int totalMinutes, int totalSeconds) {
        Platform.runLater(() -> {
            timeLabel.setText(String.format("%02d:%02d / %02d:%02d",
                    currentMinutes, currentSeconds,
                    totalMinutes, totalSeconds));
        });
    }

    /**
     * Update the seek slider position
     * @param position Position as a percentage (0-100)
     */
    public void updateSeekPosition(int position) {
        Platform.runLater(() -> {
            if (!seekSlider.isValueChanging()) {
                seekSlider.setValue(position);
            }
        });
    }

    /**
     * Update the volume slider position
     * @param volume Volume level (0-100)
     */
    public void updateVolume(int volume) {
        Platform.runLater(() -> volumeSlider.setValue(volume));
    }

    /**
     * Show file chooser dialog for track selection
     * @return List of selected files
     */
    public List<File> showFileChooser() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Select Audio Files");
        fileChooser.getExtensionFilters().add(
                new FileChooser.ExtensionFilter("Audio Files", "*.mp3", "*.wav", "*.m4a")
        );
        return fileChooser.showOpenMultipleDialog(primaryStage);
    }

    // Getters for UI controls
    public ComboBox<String> getPortList() {
        return portList;
    }

    public ToggleButton getConnectButton() {
        return connectButton;
    }

    public Button getRefreshButton() {
        return refreshButton;
    }

    public ListView<String> getPlaylistView() {
        return playlistView;
    }

    public Button getAddButton() {
        return addButton;
    }

    public Button getRemoveButton() {
        return removeButton;
    }

    public Button getClearButton() {
        return clearButton;
    }

    public Button getPrevButton() {
        return prevButton;
    }

    public Button getPlayButton() {
        return playButton;
    }

    public Button getNextButton() {
        return nextButton;
    }

    public Slider getSeekSlider() {
        return seekSlider;
    }

    public Slider getVolumeSlider() {
        return volumeSlider;
    }

    public Stage getPrimaryStage() {
        return primaryStage;
    }
}