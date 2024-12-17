//­ì©l

//#include <opencv2/opencv.hpp>
//#include <iostream>
//#include <filesystem>
//#include <string>
//
//namespace fs = std::filesystem;
//using namespace std;
//using namespace cv;
//
//void processImage(const string& input_path, const string& output_folder, int background_size, int crop_size, double rotation_angle_increment) {
//    Mat img = imread(input_path);
//    if (img.empty()) {
//        cerr << "Error: Unable to read input image: " << input_path << endl;
//        return;
//    }
//
//    // Crop the central part
//    int start_row = (img.rows - crop_size) / 2;
//    int start_col = (img.cols - crop_size) / 2;
//    Mat cropped_img = img(Rect(start_col, start_row, crop_size, crop_size));
//
//    // Create a white background
//    Mat background(background_size, background_size, img.type(), Scalar(255, 255, 255));
//
//    // Calculate the position to place the cropped image at the center of the background
//    int offset_x = (background.cols - crop_size) / 2;
//    int offset_y = (background.rows - crop_size) / 2;
//
//    // Place the cropped image at the center of the background
//    cropped_img.copyTo(background(Rect(offset_x, offset_y, crop_size, crop_size)));
//
//    // Construct the output folder path based on the input file name
//    fs::path input_path_fs(input_path);
//    string output_subfolder = output_folder + "/" + input_path_fs.stem().string();
//    fs::create_directories(output_subfolder);
//
//    // Save the rotated images with background
//    for (double angle = 0; angle < 360; angle += rotation_angle_increment) {
//        // Rotate the image clockwise
//        Mat rotation_matrix = getRotationMatrix2D(Point2f(background.cols / 2.0, background.rows / 2.0), -angle, 1);
//        Mat rotated_background;
//        warpAffine(background, rotated_background, rotation_matrix, background.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(255, 255, 255));
//
//        // Construct the output file path with original filename and angle number
//        string output_filename = input_path_fs.stem().string() + "_R" + to_string(static_cast<int>(angle)) + ".jpg";
//        string output_path = output_subfolder + "/" + output_filename;
//
//        // Save the rotated image
//        imwrite(output_path, rotated_background);
//    }
//}
//
//
//int main(int argc, char* argv[]) {
//    if (argc < 6) {
//        cerr << "Usage: " << argv[0] << " <input_folder> <output_folder> <background_size> <crop_size> <rotation_angle_increment>" << endl;
//        return 1;
//    }
//
//    string input_folder = argv[1];
//    string output_folder = argv[2];
//    int background_size = stoi(argv[3]);
//    int crop_size = stoi(argv[4]);
//    double rotation_angle_increment = stod(argv[5]);
//
//    // Ensure the output folder exists
//    fs::create_directories(output_folder);
//
//    // Process each image file in the input folder
//    for (const auto& entry : fs::directory_iterator(input_folder)) {
//        if (entry.path().extension() == ".png" || entry.path().extension() == ".jpg") {
//            processImage(entry.path().string(), output_folder, background_size, crop_size, rotation_angle_increment);
//        }
//    }
//
//    return 0;
//}
//
//

#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
using namespace std;
using namespace cv;

void processImage(const string& input_path, const string& input_folder, const string& output_folder, int background_size, int crop_size, double rotation_angle_increment) {
    Mat img = imread(input_path);
    if (img.empty()) {
        cerr << "Error: Unable to read input image: " << input_path << endl;
        return;
    }

    // Crop the central part
    int start_row = (img.rows - crop_size) / 2;
    int start_col = (img.cols - crop_size) / 2;
    Mat cropped_img = img(Rect(start_col, start_row, crop_size, crop_size));

    // Create a white background
    Mat background(background_size, background_size, img.type(), Scalar(255, 255, 255));

    // Calculate the position to place the cropped image at the center of the background
    int offset_x = (background.cols - crop_size) / 2;
    int offset_y = (background.rows - crop_size) / 2;

    // Place the cropped image at the center of the background
    cropped_img.copyTo(background(Rect(offset_x, offset_y, crop_size, crop_size)));

    // Generate output path based on the relative path from input_folder to input_path
    fs::path input_path_fs(input_path);
    string relative_path = fs::relative(input_path_fs.parent_path(), fs::path(input_folder)).string();
    string output_subfolder = output_folder + "/" + relative_path;
    fs::create_directories(output_subfolder);  // Create the directory if it doesn't exist

    // Save the rotated images with background
    for (double angle = 0; angle < 360; angle += rotation_angle_increment) {
        // Rotate the image clockwise
        Mat rotation_matrix = getRotationMatrix2D(Point2f(background.cols / 2.0, background.rows / 2.0), -angle, 1);
        Mat rotated_background;
        warpAffine(background, rotated_background, rotation_matrix, background.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(255, 255, 255));

        // Construct the output file path with original filename and angle number
        string output_filename = input_path_fs.stem().string() + "_R" + to_string(static_cast<int>(angle)) + ".jpg";
        string output_path = output_subfolder + "/" + output_filename;

        // Save the rotated image
        imwrite(output_path, rotated_background);
    }
}

void processDirectory(const string& input_folder, const string& output_folder, int background_size, int crop_size, double rotation_angle_increment) {
    for (const auto& entry : fs::recursive_directory_iterator(input_folder)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".png" || entry.path().extension() == ".jpg")) {
            processImage(entry.path().string(), input_folder, output_folder, background_size, crop_size, rotation_angle_increment);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 6) {
        cerr << "Usage: " << argv[0] << " <input_folder> <output_folder> <background_size> <crop_size> <rotation_angle_increment>" << endl;
        return 1;
    }

    string input_folder = argv[1];
    string output_folder = argv[2];
    int background_size = stoi(argv[3]);
    int crop_size = stoi(argv[4]);
    double rotation_angle_increment = stod(argv[5]);

    // Ensure the output folder exists
    fs::create_directories(output_folder);

    // Process each image file in the input folder recursively
    processDirectory(input_folder, output_folder, background_size, crop_size, rotation_angle_increment);

    return 0;
}

