package main.java.djcontroller.model;

import javafx.scene.media.Media;
import javafx.scene.media.MediaPlayer;
import javafx.util.Duration;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;

/**
 * PlaylistModel class - handles the playlist and playback of audio tracks
 */
public class PlaylistModel {
    private final List<TrackInfo> tracks = new ArrayList<>();
    private int currentTrackIndex = 0;

    private boolean isPlaying = false;
    private int currentPositionInSeconds = 0;
    private int volume = 50;
    private final AtomicBoolean volumeUpdateInProgress = new AtomicBoolean(false);

    private MediaPlayer mediaPlayer;

    private Consumer<Boolean> playStateChangeCallback;
    private Consumer<Integer> trackChangeCallback;
    private Consumer<Integer> positionChangeCallback;
    private Consumer<Integer> volumeChangeCallback;
    private Runnable beatDetectedCallback;

    private Timer beatTimer;
    private int beatsPerMinute = 120;

    private Timer arduinoSyncTimer;
    private Consumer<Integer> arduinoTimeUpdateCallback;

    /**
     * Track information class
     */
    private static class TrackInfo {
        private final String filepath;
        private final String name;
        private int durationInSeconds;
        private Media media;

        public TrackInfo(File file) {
            this.filepath = file.getAbsolutePath();
            this.name = file.getName();
            try {
                this.media = new Media(file.toURI().toString());
                this.durationInSeconds = 180;
            } catch (Exception e) {
                System.err.println("Error creating media for file: " + file.getAbsolutePath());
                e.printStackTrace();
                this.media = null;
            }
        }

        public String getFilepath() {
            return filepath;
        }

        public String getName() {
            return name;
        }

        public int getDurationInSeconds() {
            return durationInSeconds;
        }

        public void setDurationInSeconds(int durationInSeconds) {
            this.durationInSeconds = durationInSeconds;
        }

        public Media getMedia() {
            return media;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    /**
     * Constructor
     */
    public PlaylistModel() {
        setupArduinoSyncTimer();
    }

    /**
     * Add a track to the playlist
     * @param file The audio file to add
     */
    public void addTrack(File file) {
        TrackInfo track = new TrackInfo(file);
        tracks.add(track);
    }

    /**
     * Remove a track from the playlist
     * @param index The index of the track to remove
     */
    public void removeTrack(int index) {
        if (index >= 0 && index < tracks.size()) {
            if (index == currentTrackIndex && isPlaying) {
                stop();
            }

            tracks.remove(index);

            if (tracks.isEmpty()) {
                currentTrackIndex = 0;
                stop();
            } else if (index <= currentTrackIndex) {
                if (index == currentTrackIndex) {
                    if (currentTrackIndex >= tracks.size()) {
                        currentTrackIndex = tracks.size() - 1;
                    }
                    if (trackChangeCallback != null) {
                        trackChangeCallback.accept(currentTrackIndex);
                    }
                } else {
                    currentTrackIndex--;
                }
            }
        }
    }

    /**
     * Clear all tracks from the playlist
     */
    public void clearPlaylist() {
        stop();
        tracks.clear();
        currentTrackIndex = 0;
    }

    /**
     * Get the current number of tracks in the playlist
     * @return Number of tracks
     */
    public int getTrackCount() {
        return tracks.size();
    }

    /**
     * Get the track name at the specified index
     * @param index Track index
     * @return Track name or empty string if invalid index
     */
    public String getTrackName(int index) {
        if (index >= 0 && index < tracks.size()) {
            return tracks.get(index).getName();
        }
        return "";
    }

    /**
     * Get the track duration at the specified index
     * @param index Track index
     * @return Duration in seconds or 0 if invalid index
     */
    public int getTrackDuration(int index) {
        if (index >= 0 && index < tracks.size()) {
            return tracks.get(index).getDurationInSeconds();
        }
        return 0;
    }

    /**
     * Get the current track index (0-based)
     * @return Current track index
     */
    public int getCurrentTrackIndex() {
        return currentTrackIndex;
    }

    /**
     * Set the current track index
     * @param index New track index
     */
    public void setCurrentTrackIndex(int index) {
        if (index >= 0 && index < tracks.size() && index != currentTrackIndex) {
            boolean wasPlaying = isPlaying;
            stop();

            currentTrackIndex = index;
            currentPositionInSeconds = 0;

            if (trackChangeCallback != null) {
                trackChangeCallback.accept(currentTrackIndex);
            }

            if (positionChangeCallback != null) {
                positionChangeCallback.accept(currentPositionInSeconds);
            }

            if (arduinoTimeUpdateCallback != null) {
                arduinoTimeUpdateCallback.accept(currentPositionInSeconds);
            }

            if (wasPlaying) {
                play();
            }
        }
    }

    /**
     * Go to the next track in the playlist
     */
    public void nextTrack() {
        if (tracks.isEmpty()) return;

        int nextIndex = (currentTrackIndex + 1) % tracks.size();
        setCurrentTrackIndex(nextIndex);
    }

    /**
     * Go to the previous track in the playlist
     */
    public void previousTrack() {
        if (tracks.isEmpty()) return;

        if (currentPositionInSeconds > 3) {
            seek(0);
            return;
        }

        int prevIndex = (currentTrackIndex > 0) ? (currentTrackIndex - 1) : (tracks.size() - 1);
        setCurrentTrackIndex(prevIndex);
    }

    /**
     * Get the current position in seconds
     * @return Current position in seconds
     */
    public int getCurrentPositionInSeconds() {
        if (mediaPlayer != null && isPlaying) {
            return (int) mediaPlayer.getCurrentTime().toSeconds();
        }
        return currentPositionInSeconds;
    }

    /**
     * Get the current track's total duration
     * @return Duration in seconds
     */
    public int getCurrentTrackDuration() {
        if (tracks.isEmpty()) return 0;
        return tracks.get(currentTrackIndex).getDurationInSeconds();
    }

    /**
     * Get the current track's name
     * @return Track name
     */
    public String getCurrentTrackName() {
        if (tracks.isEmpty()) return "No track";
        return tracks.get(currentTrackIndex).getName();
    }

    /**
     * Seek to a position in the current track
     * @param positionInSeconds New position in seconds
     */
    public void seek(int positionInSeconds) {
        if (tracks.isEmpty()) return;

        TrackInfo currentTrack = tracks.get(currentTrackIndex);
        if (positionInSeconds >= 0 && positionInSeconds <= currentTrack.getDurationInSeconds()) {
            currentPositionInSeconds = positionInSeconds;

            // Update MediaPlayer position if playing
            if (mediaPlayer != null) {
                mediaPlayer.seek(Duration.seconds(positionInSeconds));
            }

            if (positionChangeCallback != null) {
                positionChangeCallback.accept(currentPositionInSeconds);
            }

            // Send position update to Arduino
            if (arduinoTimeUpdateCallback != null) {
                arduinoTimeUpdateCallback.accept(currentPositionInSeconds);
            }
        }
    }

    /**
     * Seek to a position by percentage
     * @param percent Position as percentage (0-100)
     */
    public void seekByPercentage(int percent) {
        if (tracks.isEmpty()) return;

        TrackInfo currentTrack = tracks.get(currentTrackIndex);
        int position = (int)(percent * currentTrack.getDurationInSeconds() / 100.0);
        seek(position);
    }

    /**
     * Seek by a relative number of seconds
     * @param relativeSeconds Number of seconds to seek (positive for forward, negative for backward)
     */
    public void seekByRelativeSeconds(int relativeSeconds) {
        if (tracks.isEmpty() || mediaPlayer == null) return;

        int currentPos = (int) mediaPlayer.getCurrentTime().toSeconds();

        int newPosition = currentPos + relativeSeconds;

        TrackInfo currentTrack = tracks.get(currentTrackIndex);
        if (newPosition < 0) {
            newPosition = 0;
        } else if (newPosition > currentTrack.getDurationInSeconds()) {
            newPosition = currentTrack.getDurationInSeconds();
        }

        seek(newPosition);
    }

    /**
     * Check if music is currently playing
     * @return True if playing, false otherwise
     */
    public boolean isPlaying() {
        return isPlaying;
    }

    /**
     * Start playback
     */
    public void play() {
        if (tracks.isEmpty()) return;

        if (!isPlaying) {
            TrackInfo currentTrack = tracks.get(currentTrackIndex);

            if (mediaPlayer == null) {
                if (currentTrack.getMedia() != null) {
                    try {
                        mediaPlayer = new MediaPlayer(currentTrack.getMedia());
                        setupMediaPlayer();
                    } catch (Exception e) {
                        System.err.println("Error creating MediaPlayer: " + e.getMessage());
                        e.printStackTrace();
                        return;
                    }
                } else {
                    System.err.println("Cannot play track: Media is null");
                    return;
                }
            }

            mediaPlayer.play();
            isPlaying = true;

            if (playStateChangeCallback != null) {
                playStateChangeCallback.accept(true);
            }

            startBeatDetection();
            startArduinoSync();
        }
    }

    /**
     * Set up MediaPlayer event handlers and properties
     */
    private void setupMediaPlayer() {
        if (mediaPlayer == null) return;

        mediaPlayer.setVolume(volume / 100.0);

        if (currentPositionInSeconds > 0) {
            mediaPlayer.setStartTime(Duration.seconds(currentPositionInSeconds));
        }

        mediaPlayer.setOnReady(() -> {
            Duration duration = mediaPlayer.getMedia().getDuration();
            int durationSeconds = (int) duration.toSeconds();

            TrackInfo currentTrack = tracks.get(currentTrackIndex);
            currentTrack.setDurationInSeconds(durationSeconds);

            if (positionChangeCallback != null) {
                positionChangeCallback.accept(currentPositionInSeconds);
            }

            if (arduinoTimeUpdateCallback != null) {
                arduinoTimeUpdateCallback.accept(currentPositionInSeconds);
            }

            if (currentPositionInSeconds > 0) {
                mediaPlayer.seek(Duration.seconds(currentPositionInSeconds));
            }
        });

        mediaPlayer.setOnEndOfMedia(this::nextTrack);

        mediaPlayer.currentTimeProperty().addListener((obs, oldTime, newTime) -> {
            int oldSeconds = (int) oldTime.toSeconds();
            int newSeconds = (int) newTime.toSeconds();

            if (newSeconds != oldSeconds) {
                currentPositionInSeconds = newSeconds;

                if (positionChangeCallback != null) {
                    positionChangeCallback.accept(currentPositionInSeconds);
                }
            }
        });

        mediaPlayer.setOnError(() -> {
            System.err.println("Media error: " + mediaPlayer.getError().toString());
            stop();
        });
    }

    /**
     * Pause playback
     */
    public void pause() {
        if (isPlaying && mediaPlayer != null) {
            mediaPlayer.pause();
            isPlaying = false;

            if (playStateChangeCallback != null) {
                playStateChangeCallback.accept(false);
            }

            stopBeatDetection();
            stopArduinoSync();
        }
    }

    /**
     * Stop playback
     */
    public void stop() {
        if (mediaPlayer != null) {
            currentPositionInSeconds = (int) mediaPlayer.getCurrentTime().toSeconds();

            mediaPlayer.stop();
            mediaPlayer.dispose();
            mediaPlayer = null;
        }

        isPlaying = false;

        if (playStateChangeCallback != null) {
            playStateChangeCallback.accept(false);
        }

        stopBeatDetection();
        stopArduinoSync();
    }

    /**
     * Set up the Arduino sync timer
     */
    private void setupArduinoSyncTimer() {
        arduinoSyncTimer = new Timer("ArduinoTimeSync", true);
    }

    /**
     * Start Arduino time sync
     * Sends regular time updates to the Arduino
     */
    private void startArduinoSync() {
        if (arduinoSyncTimer != null) {
            arduinoSyncTimer.cancel();
        }

        arduinoSyncTimer = new Timer("ArduinoTimeSync", true);
        arduinoSyncTimer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if (arduinoTimeUpdateCallback != null && isPlaying) {
                    int currentPos = getCurrentPositionInSeconds();
                    arduinoTimeUpdateCallback.accept(currentPos);
                }
            }
        }, 0, 1000);
    }

    /**
     * Stop Arduino time sync
     */
    private void stopArduinoSync() {
        if (arduinoSyncTimer != null) {
            arduinoSyncTimer.cancel();
            arduinoSyncTimer = null;
        }
    }

    /**
     * Get the volume level
     * @return Volume level (0-100)
     */
    public int getVolume() {
        return volume;
    }

    /**
     * Set the volume level - fixed to prevent feedback loops
     * @param level Volume level (0-100)
     */
    public void setVolume(int level) {
        if (volumeUpdateInProgress.getAndSet(true)) {
            return;
        }

        try {
            if (level >= 0 && level <= 100) {
                volume = level;

                if (mediaPlayer != null) {
                    mediaPlayer.setVolume(volume / 100.0);
                }

                if (volumeChangeCallback != null) {
                    volumeChangeCallback.accept(volume);
                }
            }
        } finally {
            volumeUpdateInProgress.set(false);
        }
    }

    /**
     * Set the callback for play state changes
     * @param callback Consumer that takes a boolean (isPlaying)
     */
    public void setPlayStateChangeCallback(Consumer<Boolean> callback) {
        this.playStateChangeCallback = callback;
    }

    /**
     * Set the callback for track changes
     * @param callback Consumer that takes the new track index
     */
    public void setTrackChangeCallback(Consumer<Integer> callback) {
        this.trackChangeCallback = callback;
    }

    /**
     * Set the callback for position changes
     * @param callback Consumer that takes the new position in seconds
     */
    public void setPositionChangeCallback(Consumer<Integer> callback) {
        this.positionChangeCallback = callback;
    }

    /**
     * Set the callback for volume changes
     * @param callback Consumer that takes the new volume level (0-100)
     */
    public void setVolumeChangeCallback(Consumer<Integer> callback) {
        this.volumeChangeCallback = callback;
    }

    /**
     * Set the callback for beat detection
     * @param callback Runnable to be called when a beat is detected
     */
    public void setBeatDetectedCallback(Runnable callback) {
        this.beatDetectedCallback = callback;
    }

    /**
     * Start the beat detection simulation
     */
    private void startBeatDetection() {
        if (beatTimer != null) {
            beatTimer.cancel();
        }

        beatTimer = new Timer("BeatDetection", true);
        long interval = (long)(60000.0 / beatsPerMinute);

        beatTimer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if (beatDetectedCallback != null) {
                    beatDetectedCallback.run();
                }
            }
        }, 0, interval);
    }

    /**
     * Stop the beat detection simulation
     */
    private void stopBeatDetection() {
        if (beatTimer != null) {
            beatTimer.cancel();
            beatTimer = null;
        }
    }
}