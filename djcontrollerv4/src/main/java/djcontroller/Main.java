package main.java.djcontroller;

import javafx.application.Application;
import javafx.scene.image.Image;
import javafx.stage.Stage;

import main.java.djcontroller.model.ArduinoModel;
import main.java.djcontroller.model.PlaylistModel;
import main.java.djcontroller.view.DJControllerPresenter;
import main.java.djcontroller.view.DJControllerView;

/**
 * Main application class for the DJ Controller
 * Sets up the Model-View-Presenter structure and starts the application
 */
public class Main extends Application {

    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("DJ Controller");

        primaryStage.setMinWidth(800);
        primaryStage.setMinHeight(600);

        PlaylistModel playlistModel = new PlaylistModel();
        ArduinoModel arduinoModel = new ArduinoModel();

        DJControllerView view = new DJControllerView(primaryStage);

        new DJControllerPresenter(view, playlistModel, arduinoModel);
    }

    /**
     * Application entry point
     * @param args Command line arguments
     */
    public static void main(String[] args) {
        System.setProperty("javafx.graphics.pulseID", "current");
        System.setProperty("javafx.audio.size", "4096");

        launch(args);
    }
}