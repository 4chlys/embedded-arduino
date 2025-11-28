package main.java.djcontroller.model;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortDataListener;
import com.fazecast.jSerialComm.SerialPortEvent;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

/**
 * Model class - handles Arduino communication with single-byte commands
 */
public class ArduinoModel {
    private SerialPort comPort;
    private OutputStream output;
    private InputStream input;
    private boolean isConnected = false;

    private Runnable playHandler;
    private Runnable pauseHandler;
    private Runnable nextTrackHandler;
    private Runnable prevTrackHandler;
    private Consumer<Integer> seekHandler;
    private Runnable statusRequestHandler;

    private Consumer<String> statusChangeCallback;

    private boolean debugMode = true;

    private boolean lastSentPlayingState = false;
    private int lastSentCurrentTrack = 1;
    private int lastSentTotalTracks = 1;

    /**
     * Get a list of available serial ports
     * @return List of port names
     */
    public List<String> getAvailablePorts() {
        List<String> portNames = new ArrayList<>();
        for (SerialPort port : SerialPort.getCommPorts()) {
            portNames.add(port.getSystemPortName());
        }
        return portNames;
    }

    /**
     * Connect to a serial port
     * @param portName Name of the port to connect to
     * @return True if successful, false otherwise
     */
    public boolean connect(String portName) {
        if (isConnected) {
            disconnect();
        }

        comPort = SerialPort.getCommPort(portName);
        comPort.setComPortParameters(9600, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);
        comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 100, 0);

        if (comPort.openPort()) {
            isConnected = true;
            output = comPort.getOutputStream();
            input = comPort.getInputStream();

            setupDataListener();

            if (statusChangeCallback != null) {
                statusChangeCallback.accept("Connected to " + portName);
            }

            try {
                Thread.sleep(2000); // Give Arduino time to boot
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }

            // Reset tracking variables
            lastSentPlayingState = false;
            lastSentCurrentTrack = 1;
            lastSentTotalTracks = 1;

            return true;
        } else {
            if (statusChangeCallback != null) {
                statusChangeCallback.accept("Failed to connect to " + portName);
            }
            return false;
        }
    }

    /**
     * Disconnect from the serial port
     */
    public void disconnect() {
        if (comPort != null && comPort.isOpen()) {
            comPort.closePort();
            isConnected = false;

            if (statusChangeCallback != null) {
                statusChangeCallback.accept("Disconnected");
            }
        }
    }

    /**
     * Check if connected to Arduino
     * @return True if connected, false otherwise
     */
    public boolean isConnected() {
        return isConnected;
    }

    /**
     * Set up the serial port data listener
     */
    private void setupDataListener() {
        comPort.addDataListener(new SerialPortDataListener() {
            @Override
            public int getListeningEvents() {
                return SerialPort.LISTENING_EVENT_DATA_AVAILABLE;
            }

            @Override
            public void serialEvent(SerialPortEvent event) {
                if (event.getEventType() != SerialPort.LISTENING_EVENT_DATA_AVAILABLE)
                    return;

                byte[] newData = new byte[comPort.bytesAvailable()];
                comPort.readBytes(newData, newData.length);

                processArduinoCommands(newData);
            }
        });
    }

    /**
     * Process commands received from Arduino (single bytes)
     * @param data Command data
     */
    private void processArduinoCommands(byte[] data) {
        if (data.length == 0) return;

        for (byte b : data) {
            char command = (char) b;

            debugLog("Received command: '" + command + "'");

            switch (command) {
                case 'P': // Play request from Arduino
                    debugLog("Arduino requests: PLAY");
                    if (playHandler != null) {
                        playHandler.run();
                    }
                    break;

                case 'S': // Pause request from Arduino
                    debugLog("Arduino requests: PAUSE");
                    if (pauseHandler != null) {
                        pauseHandler.run();
                    }
                    break;

                case 'N': // Next track request from Arduino
                    debugLog("Arduino requests: NEXT TRACK");
                    if (nextTrackHandler != null) {
                        new Thread(() -> {
                            nextTrackHandler.run();
                        }).start();
                    }
                    break;

                case 'B': // Previous track request from Arduino
                    debugLog("Arduino requests: PREVIOUS TRACK");
                    if (prevTrackHandler != null) {
                        new Thread(() -> {
                            prevTrackHandler.run();
                        }).start();
                    }
                    break;

                case 'F': // Forward seek request from Arduino
                    debugLog("Arduino requests: SEEK FORWARD");
                    if (seekHandler != null) {
                        seekHandler.accept(30); // Forward 30 seconds
                    }
                    break;

                case 'R': // Backward seek request from Arduino
                    debugLog("Arduino requests: SEEK BACKWARD");
                    if (seekHandler != null) {
                        seekHandler.accept(-30); // Backward 30 seconds
                    }
                    break;

                case 'Q': // Status request from Arduino
                    debugLog("Arduino requests: STATUS UPDATE");
                    if (statusRequestHandler != null) {
                        statusRequestHandler.run();
                    }
                    break;

                default:
                    debugLog("Unknown command from Arduino: '" + command + "'");
                    break;
            }
        }
    }

    /**
     * Send a single byte command to Arduino
     * @param command Single character command
     */
    private void sendSingleCommand(char command) {
        if (!isConnected) return;

        try {
            output.write(command);
            output.flush();
            Thread.sleep(10); // Small delay between commands
            debugLog("Sent command to Arduino: '" + command + "'");
        } catch (IOException e) {
            handleSendError("command '" + command + "'", e);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    /**
     * Send playing status to Arduino
     * @param isPlaying Whether the track is playing
     */
    public void sendPlayStatus(boolean isPlaying) {
        if (!isConnected || isPlaying == lastSentPlayingState) return;

        if (isPlaying) {
            sendSingleCommand('P');
        } else {
            sendSingleCommand('S');
        }

        lastSentPlayingState = isPlaying;
        debugLog("Sent play status: " + (isPlaying ? "PLAYING" : "PAUSED"));
    }

    /**
     * Send track information to Arduino using incremental commands
     * @param currentTrack Current track number (1-based)
     * @param totalTracks Total number of tracks
     */
    public void sendTrackInfo(int currentTrack, int totalTracks) {
        if (!isConnected) return;

        debugLog("Updating Arduino track info: " + currentTrack + "/" + totalTracks +
                " (was: " + lastSentCurrentTrack + "/" + lastSentTotalTracks + ")");

        while (lastSentTotalTracks < totalTracks) {
            sendSingleCommand('T');
            lastSentTotalTracks++;
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            }
        }

        while (lastSentTotalTracks > totalTracks) {
            sendSingleCommand('D');
            lastSentTotalTracks--;
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            }
        }

        while (lastSentCurrentTrack < currentTrack) {
            sendSingleCommand('C');
            lastSentCurrentTrack++;
            if (lastSentCurrentTrack > lastSentTotalTracks) {
                lastSentCurrentTrack = 1;
            }
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            }
        }

        while (lastSentCurrentTrack > currentTrack) {
            sendSingleCommand('V');
            lastSentCurrentTrack--;
            if (lastSentCurrentTrack < 1) {
                lastSentCurrentTrack = lastSentTotalTracks;
            }
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            }
        }

        debugLog("Arduino track info updated successfully");
    }

    /**
     * Send track change notification (when track changes in Java)
     * @param isNext True for next track, false for previous
     * @param currentlyPlaying Current playing state after track change
     */
    public void sendTrackChange(boolean isNext, boolean currentlyPlaying) {
        if (!isConnected) return;

        if (isNext) {
            sendSingleCommand('N');
            lastSentCurrentTrack++;
            if (lastSentCurrentTrack > lastSentTotalTracks) {
                lastSentCurrentTrack = 1;
            }
        } else {
            sendSingleCommand('B');
            lastSentCurrentTrack--;
            if (lastSentCurrentTrack < 1) {
                lastSentCurrentTrack = lastSentTotalTracks;
            }
        }

        try {
            Thread.sleep(50);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        lastSentPlayingState = !currentlyPlaying;
        sendPlayStatus(currentlyPlaying);

        debugLog("Sent track change: " + (isNext ? "NEXT" : "PREVIOUS") + " with play state: " + currentlyPlaying);
    }

    /**
     * Send beat detected signal to Arduino
     */
    public void sendBeatDetected() {
        if (!isConnected) return;
        sendSingleCommand('b');
    }

    /**
     * Send status request to Arduino
     */
    public void requestStatus() {
        if (!isConnected) return;
        sendSingleCommand('Q');
    }

    /**
     * Reset Arduino track counters (for new playlist)
     */
    public void resetTrackCounters() {
        lastSentCurrentTrack = 1;
        lastSentTotalTracks = 1;
        lastSentPlayingState = false;
        debugLog("Reset track counters to defaults");
    }

    /**
     * Send full status update to Arduino (use when unsure of sync state)
     */
    public void sendFullStatusUpdate(boolean isPlaying, int currentTrack, int totalTracks) {
        if (!isConnected) return;

        debugLog("Sending full status update: " + isPlaying + ", track " + currentTrack + "/" + totalTracks);

        lastSentPlayingState = !isPlaying;

        sendTrackInfo(currentTrack, totalTracks);

        sendPlayStatus(isPlaying);
    }

    /**
     * Handle send error
     * @param type Type of data being sent
     * @param e Exception
     */
    private void handleSendError(String type, IOException e) {
        debugLog("Failed to send " + type + ": " + e.getMessage());
        if (!comPort.isOpen()) {
            isConnected = false;
            if (statusChangeCallback != null) {
                statusChangeCallback.accept("Lost connection to Arduino");
            }
        }
    }

    /**
     * Log debug message
     * @param message Debug message
     */
    private void debugLog(String message) {
        if (debugMode) {
            System.out.println("[ArduinoModel] " + message);
        }
    }

    public void setPlayHandler(Runnable handler) {
        this.playHandler = handler;
    }

    public void setPauseHandler(Runnable handler) {
        this.pauseHandler = handler;
    }

    public void setNextTrackHandler(Runnable handler) {
        this.nextTrackHandler = handler;
    }

    public void setPrevTrackHandler(Runnable handler) {
        this.prevTrackHandler = handler;
    }

    public void setSeekHandler(Consumer<Integer> handler) {
        this.seekHandler = handler;
    }

    public void setStatusRequestHandler(Runnable handler) {
        this.statusRequestHandler = handler;
    }

    public void setStatusChangeCallback(Consumer<String> callback) {
        this.statusChangeCallback = callback;
    }
}