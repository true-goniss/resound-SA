#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

#include <filesystem>
#include "utils.h"

namespace fs = std::filesystem;

bool onlyLocalIcon = false;

inline void processIconImage(const std::string& outputPath, const std::string& iconPath, const std::string& iconName, bool useInitialName) {
    cv::Mat im = cv::imread(iconPath, cv::IMREAD_UNCHANGED);

    if (im.empty()) {
        return; 
        // Exit if image not loaded
    }

    std::string outputIconPath = useInitialName
        ? (fs::path(outputPath) / iconName).replace_extension("").string() + "_round.png"
        : (fs::path(outputPath) / "icon_round.png").string();

    // Ensure image has alpha channel (RGBA)
    if (im.channels() != 4) {
        cv::cvtColor(im, im, cv::COLOR_BGR2BGRA);
    }

    // Create a circular mask
    int minDim = std::min(im.cols, im.rows);
    cv::Mat mask(im.size(), CV_8UC1, cv::Scalar(0));
    cv::circle(mask, cv::Point(im.cols / 2, im.rows / 2), minDim / 2, cv::Scalar(255), cv::FILLED);

    // Apply the circular mask to the image
    cv::Mat cropped(im.size(), CV_8UC4, cv::Scalar(0, 0, 0, 0)); // Transparent background
    im.copyTo(cropped, mask);

    // Crop to the bounding box of the circle
    cv::Rect boundingBox((im.cols - minDim) / 2, (im.rows - minDim) / 2, minDim, minDim);
    cv::Mat tightCropped = cropped(boundingBox);

    cv::Mat grayIcon;
    cv::cvtColor(tightCropped, grayIcon, cv::COLOR_BGRA2GRAY);

    // Reconstruct as RGBA with grayscale applied
    std::vector<cv::Mat> channels(4);
    cv::split(tightCropped, channels); // Split original RGBA channels
    channels[0] = grayIcon;            // Set blue channel to grayscale
    channels[1] = grayIcon;            // Set green channel to grayscale
    channels[2] = grayIcon;            // Set red channel to grayscale
    cv::merge(channels, tightCropped); // Merge back into RGBA format

    // Resize to 150x150
    cv::Mat resizedIcon;
    cv::resize(tightCropped, resizedIcon, cv::Size(150, 150), 0, 0, cv::INTER_AREA);

    // Save the processed icon
    cv::imwrite(outputIconPath, resizedIcon);
}



inline void process() {
    
    fs::path currentPath = fs::current_path();

    if (onlyLocalIcon) { 

        processIconImage(
            "icon.png", 
            currentPath.string(), 
            "", 
            false
        );

        return; 
    }

    fs::path radiosPath = (currentPath.parent_path()) / "radio";

    for (const auto& entry : fs::directory_iterator(radiosPath)) {
        if (entry.is_directory()) {
            fs::path iconPath = entry.path().string() +  "\\icon.png";

            if (fs::exists(iconPath) && fs::is_regular_file(iconPath)) {
                processIconImage(entry.path().string(), iconPath.string(), "", false);
            }
        }
    };

    fs::path radio_icons_path = (currentPath.parent_path()) / "textures" / "radio_icons";

    for (const auto& entry : fs::directory_iterator(radio_icons_path)) {
        if (entry.is_regular_file() && fs::exists(entry.path())) {
                
            if (Utils::ContainsCaseInsensitive(entry.path().filename().string(), "round")) continue;

            processIconImage(radio_icons_path.string(), entry.path().string(), entry.path().filename().string(), true);
        }
    };
}

int main();

















//cv::Mat image = im;
//
//// Create a circular mask with a black circle
//cv::Mat mask(image.size(), CV_8UC1, cv::Scalar(0)); // Initialize mask with zeros (black)
//cv::circle(mask, cv::Point(image.cols / 2, image.rows / 2), std::min(image.cols, image.rows) * 0.35, cv::Scalar(255), cv::FILLED); // Set circular region to full intensity (white)
//
//// Create a transparent black canvas
//cv::Mat black_canvas(image.size(), CV_8UC4, cv::Scalar(0, 0, 0, 0)); // Initialize with transparent black
//
//// Copy the circular region from the original image to the black canvas
//
//
//// Set alpha channel to 0 for pixels outside the circular region
//for (int y = 0; y < mask.rows; ++y) {
//    for (int x = 0; x < mask.cols; ++x) {
//        if (mask.at<uchar>(y, x) == 0) {
//
//
//        }
//        else {
//            black_canvas.at<cv::Vec4b>(y, x)[0] = 0; // Set alpha channel to 0
//            black_canvas.at<cv::Vec4b>(y, x)[1] = 0; // Set alpha channel to 0
//            black_canvas.at<cv::Vec4b>(y, x)[2] = 0; // Set alpha channel to 0
//            black_canvas.at<cv::Vec4b>(y, x)[3] = 255; // Set alpha channel to 0
//        }
//    }
//}
//
//image.copyTo(black_canvas, mask);