/*
    Author: Briggs Richardson
    opencv-4.0.0 console driven image/webcam manipulation program

    The program uses Mat images (opencv data types to store images)
    and then uses given opencv functions to iterate through given
    images to manipulate each pixel's RGB values for desired features
    such as grayscale, black and white, strobel outline, motion detection,
    and more.
*/

#include "opencv2/opencv.hpp"
#include <iostream>
#include <limits>
#include <cmath>
#include <fstream>
#include <string>

/* Function declarations -- get inputs from user */
int getModeInput();
std::string getImageChoice();
int displayMenu(int mode);
void getManipulationSpecifications(int choice);

/* Function declaration -- execute a chosen manipulation */
void executeManipulation(int choice, int mode);

/* Function declarations -- manipulation functions */
void originalMedia();
void blackWhite();
void grayscale();
void darken();
void rgbPercentages();
void purify();
void strobelOutline();
void approximate();
void motionDetection();

/* Function declarations -- helper functions for manipulations */
double getLuminosity(double b, double g, double r);
int isClose(float originalB, float originalG, float originalR,
            float b, float g, float r, int strength);
double getDistance(float originalBlue, float originalGreen, float originalRed,
                   float blue, float green, float red);
int smallest(double up, double right, double down, double left);

/* Function declarations -- Sanitized input to avoid breaking the program */
int getSanitizedInt(const std::string prompt, int lower, int upper);
double getSanitizedDouble(const std::string prompt, int lower, int upper);

/* Resize image/window to given H/W to fit on screen (fill free to change */
int WIDTH = 550;
int HEIGHT = 350;

/* Mat objects to store data (pixels) on the images (png only) */
cv::Mat original;
cv::Mat prevFrame;
cv::Mat modified;

/* Given manipulation specifications for some of the features */
int bwThreshold = 0;
double brightnessConstant;
double redMult, blueMult, greenMult;

int main() {
    int mode;
    std::string imageName;
    int manipulationChoice = 0;

    int IMAGE_MODE = 1;
    int WEBCAM_MODE = 2;
    int QUIT = 8;
    int APPROXIMATE = 7;
    int MOTION_DETECTION = 7;

    cv::namedWindow("Modified", cv::WINDOW_FREERATIO);  // Display window

    std::cout << "Image/Webcam Manipulation Program" << std::endl;
    std::cout << "---------------------------------" << std::endl << std::endl;

    // Ask user if they want to manipulate an image or webcam
    mode = getModeInput();   // 1 = IMAGE_MODE, 2 = WEBCAM_MODE

    /* If user wants to manipulate an image, ask for image  & set it up */
    if (mode == IMAGE_MODE) {
        imageName = getImageChoice();
        if (imageName.empty()) {
            std::cout << "Error loading image" << std::endl;
            return -1;
        }

        original = cv::imread("images/" + imageName, cv::IMREAD_COLOR); 
        modified = cv::imread("images/" + imageName, cv::IMREAD_COLOR); 
        cv::resize(original, original, cv::Size(WIDTH, HEIGHT));    
        cv::resize(modified, modified, cv::Size(WIDTH, HEIGHT));   
    } 
    
    /* Set up display windows */
    cv::resizeWindow("Modified", WIDTH, HEIGHT);
    cv::moveWindow("Modified", 210 + WIDTH, 0);
    
    cv::VideoCapture cap;  // To capture webcam media

    /* Main menu control loop */
    while (manipulationChoice != QUIT) {
        manipulationChoice = displayMenu(mode);
        if (manipulationChoice == QUIT)  // If QUIT, break out of loop
            break;

        if (manipulationChoice == APPROXIMATE && mode == 1) {
            std::cout << "Press ESC while focused on display to exit "
                << "approximation early" << std::endl;
        }
        
        getManipulationSpecifications(manipulationChoice);
        std::cout << "Press ESC while focused on the display to return "
                  <<  "to the main menu" << std::endl << std::endl;

        if (mode == IMAGE_MODE) {
            executeManipulation(manipulationChoice, mode);
            cv::imshow("Modified", modified);
            cv::waitKey(0);
        } else {
            if (!cap.open(0))  // Can't open webcam, exit
                return -1;
            while (true) {
                cap >> original;
                if (original.empty()) // No more feed
                    break;
                if (modified.empty()) // first frame, fill modifed as well
                    cap >> modified;

                executeManipulation(manipulationChoice, mode);
                cv::imshow("Modified", modified);
                if (cv::waitKey(10) == 27) {
                    cap.release();
                    break;
                }
            }
        }
        cv::destroyAllWindows(); 
    }

    return 0;
}


/* Asks the user if they want to manipulate an image or a webcam, returns 1
   for image manipulation and 2 for webcam manipulation */
int getModeInput() {
    int choice;
    std::string prompt = "Please enter the corresponding number to select a ";
    std::string prompt2 = "media to manipulate\n1) Image\n2) Webcam";
    std::string finalPrompt = prompt + prompt2;
    choice = getSanitizedInt(finalPrompt, 1, 2);

    return choice;
}


/* List the names of all the image files in bin/imageNames.txt and return
   the user's choice of the (string) image choice for manipulation
   Returns empty string if imageNames.txt is not found or no image names
   are available */
std::string getImageChoice() {
    std::string imageName;
    std::string imageNameChoice;
    int lineNumber = 0;
    int lineNumberChoice;

    std::ifstream imageNamesFile("imageNames.txt");
    if (!imageNamesFile.is_open()) {
        std::cout << "Error opening image names file " << std::endl;
        return imageName;  // Not assigned yet, so it will be empty (error)
    }

    std::cout << std::endl;
    std::cout << "Available images to manipulate" << std::endl;

    while (std::getline(imageNamesFile, imageName)) {
        ++lineNumber;
        std::cout << lineNumber << ") " << imageName << std::endl;
    }
    std::cout << std::endl;

    // If lineNumber is never incremented, therefore no lines of text in file
    if (lineNumber == 0) {  
        std::cout << "No image names in imageNames.txt" << std::endl;
        return imageNameChoice;  // Not assigned yet, so it will be empty (error)
    }

    lineNumberChoice = getSanitizedInt(
        "Please enter corresponding number to select an image", 1, lineNumber);

    imageNamesFile.clear();               // Clear stream
    imageNamesFile.seekg(0, std::ios::beg);    // Start from beginning of file
    
    // Search through file again for chosen line number
    lineNumber = 0;
    while(lineNumber != lineNumberChoice) {
        ++lineNumber;
        getline(imageNamesFile, imageNameChoice);
    }

    imageNamesFile.close();
    return imageName;
}


int getSanitizedInt(const std::string prompt, int lowerLimit, int upperLimit) {
    int choice;
    while (true) {
        std::cout << prompt << std::endl;
        std::cout << "Choice: ";
        std::cin >>  choice; 

        if (std::cin.fail()) {
            std::cout << "Sorry, couldn't read the input. Please try again"
                      << std::endl << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (choice < lowerLimit || choice > upperLimit) {
            std::cout << "Please enter a valid number" << std::endl << std::endl;
            continue;
        }
        break;
    }

    return choice;
}


double getSanitizedDouble(const std::string prompt, int lowerLimit, int upperLimit) {
    double choice;
    while (true) {
        std::cout << prompt << std::endl;
        std::cout << "Choice: ";
        std::cin >>  choice; 

        if (std::cin.fail()) {
            std::cout << "Sorry, couldn't read the input. Please try again"
                      << std::endl << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (choice < lowerLimit || choice > upperLimit) {
            std::cout << "Please enter a valid number" << std::endl << std::endl;
            continue;
        }
        break;
    }

    return choice;
}


/* Displays the menu and returns a sanitized menu choice (int) */
int displayMenu(int mode) {
    int choice;
    std::cout << std::endl;
    std::cout << "Main Menu" << std::endl;
    std::cout << "---------" << std::endl;
    std::cout << "0) Original" << std::endl;
    std::cout << "1) Black and White" << std::endl;
    std::cout << "2) Grayscale" << std::endl;
    std::cout << "3) Darken" << std::endl;
    std::cout << "4) RGB values" << std::endl;
    std::cout << "5) Purify RGB" << std::endl;
    std::cout << "6) Strobel Outline"  << std::endl;
    if (mode == 1)
        std::cout << "7) Approximate (Image mode only)" << std::endl;
    else 
        std::cout << "7) Motion Detection (Video mode only)" << std::endl;
    std::cout << "8) Quit" << std::endl;

    choice = getSanitizedInt("\nEnter a manipulation choice", 0, 8);
    std::cout << std::endl; // Spacing

    return choice;
}


void getManipulationSpecifications(int menuChoice) {
    switch (menuChoice) {
        case 1: // black and white
            bwThreshold = getSanitizedInt(
                    "Please enter a threshold (0-255): ", 0, 255);
            break;
        case 3: // Brightness
            brightnessConstant = getSanitizedDouble(
                "Please enter a brightness constant between 0-1: ", 0, 1);
            break;
        case 4:  // RGB values
            redMult = getSanitizedInt(
                    "Please enter a red multiplier (%): ", 0, 150);
            greenMult = getSanitizedInt(
                    "Please enter a green multiplier (%): ", 0, 150);
            blueMult = getSanitizedInt(
                    "Please enter a blue multiplier (%): ", 0, 150);
            break;
        default:
            break;
    }
}


void executeManipulation(int menuChoice, int mode) {
    switch (menuChoice) {
        case 0:
            originalMedia();
            break;
        case 1:
            blackWhite();
            break;
        case 2:
            grayscale();
            break;
        case 3:
            darken();
            break;
        case 4:
            rgbPercentages();
            break;
        case 5:
            purify();
            break;
        case 6:
            strobelOutline();
            break;
        case 7:
            if (mode == 1)
                approximate();
            else
                motionDetection();
            break;
        default:
            break;
    }
}


void originalMedia() {
    for (int r = 0; r < original.rows; r++) {
        for (int c = 0; c < original.cols; c++) {
            modified.at<cv::Vec3b>(r, c)[0] = original.at<cv::Vec3b>(r, c)[0];
            modified.at<cv::Vec3b>(r, c)[1] = original.at<cv::Vec3b>(r, c)[1];
            modified.at<cv::Vec3b>(r, c)[2] = original.at<cv::Vec3b>(r, c)[2];
        } 
    }
}


/* Looks at each pixel of the original image, determines if the RGB components 
 * surpass a given threshold, and makes the modified equivalent image either
 * black or white */ 
void blackWhite() {
    for (int r = 0; r < modified.rows; ++r) {
		for (int c = 0; c < modified.cols; ++c) {
			float blue = original.at<cv::Vec3b>(r, c)[0];
			float green = original.at<cv::Vec3b>(r, c)[1];
			float red = original.at<cv::Vec3b>(r, c)[2];
			if (((double)blue + (double)green + (double)red) / 3.0 > bwThreshold) {
				modified.at<cv::Vec3b>(r, c)[0] = 255;
				modified.at<cv::Vec3b>(r, c)[1] = 255;
				modified.at<cv::Vec3b>(r, c)[2] = 255;
			}
			else {
				modified.at<cv::Vec3b>(r, c)[0] = 0;
				modified.at<cv::Vec3b>(r, c)[1] = 0;
				modified.at<cv::Vec3b>(r, c)[2] = 0;
			}
		}
	}
}


/* Looks at each pixel of the original image and converts the pixel to grayscale
 * using grayscale formula found online */
void grayscale() {
    for (int r = 0; r < original.rows; ++r) {                           
        for (int c = 0; c < original.cols; ++c) {                       
            float blue = original.at<cv::Vec3b>(r, c)[0];               
            float green = original.at<cv::Vec3b>(r, c)[1];              
            float red = original.at<cv::Vec3b>(r, c)[2];                

            double gray = .299 * red + .587 * green + .114 * blue;      
            modified.at<cv::Vec3b>(r, c)[0] = gray;                     
            modified.at<cv::Vec3b>(r, c)[1] = gray;                     
            modified.at<cv::Vec3b>(r, c)[2] = gray;                     
        }                                                               
    }       
}


/* Multiplies the pixels by a given constant (between 0-1) to affect brightness */
void darken() {
    for (int r = 0; r < original.rows; ++r) {                           
        for (int c = 0; c < original.cols; ++c) {                       
            modified.at<cv::Vec3b>(r, c)[0] = original.at<cv::Vec3b>(r, c)[0] * brightnessConstant;
            modified.at<cv::Vec3b>(r, c)[1] = original.at<cv::Vec3b>(r, c)[1] * brightnessConstant;
            modified.at<cv::Vec3b>(r, c)[2] = original.at<cv::Vec3b>(r, c)[2] * brightnessConstant;
        }                                                               
    }    
}


/* Multiplies the pixels by a given constant (0-100) to affect percentage */
void rgbPercentages() {
    for (int r = 0; r < original.rows; ++r) {                           
        for (int c = 0; c < original.cols; ++c) {                       
            modified.at<cv::Vec3b>(r, c)[0] = original.at<cv::Vec3b>(r, c)[0] * (blueMult / 100.0);
            modified.at<cv::Vec3b>(r, c)[1] = original.at<cv::Vec3b>(r, c)[1] * (greenMult / 100.0);
            modified.at<cv::Vec3b>(r, c)[2] = original.at<cv::Vec3b>(r, c)[2] * (redMult / 100.0);
        }                                                               
    }    
}


/* Finds the highest RGB component and maxes it for the given pixel */
void purify() {
    for (int r = 0; r < original.rows; ++r) {
		for (int c = 0; c < original.cols; ++c) {
			float blue = original.at<cv::Vec3b>(r, c)[0];
			float green = original.at<cv::Vec3b>(r, c)[1];
			float red = original.at<cv::Vec3b>(r, c)[2];

			if (blue > green && blue > red) {
				modified.at<cv::Vec3b>(r, c)[0] = 255;
				modified.at<cv::Vec3b>(r, c)[1] = 0;
				modified.at<cv::Vec3b>(r, c)[2] = 0;
			}
			else if (green > blue && green > red) {
				modified.at<cv::Vec3b>(r, c)[0] = 0;
				modified.at<cv::Vec3b>(r, c)[1] = 255;
				modified.at<cv::Vec3b>(r, c)[2] = 0;
			}
			else {
				modified.at<cv::Vec3b>(r, c)[0] = 0;
				modified.at<cv::Vec3b>(r, c)[1] = 0;
				modified.at<cv::Vec3b>(r, c)[2] = 255;
			}
		}
	}
}


/* Using given strobel outline algorithm online, detects images using gradients */
void strobelOutline() {
    for (int r = 0; r < original.rows; ++r) {
        for (int c = 0; c < original.cols; ++c) {
            if (r > 1 && r < original.rows - 1 && c > 1 && c < original.cols - 1) {

                // Vertical
                double vertY;
                vertY = getLuminosity(original.at<cv::Vec3b>(r + 1, c + 1)[0], original.at<cv::Vec3b>(r + 1, c + 1)[1], original.at<cv::Vec3b>(r + 1, c + 1)[2])
                    + getLuminosity(original.at<cv::Vec3b>(r, c + 1)[0], original.at<cv::Vec3b>(r, c + 1)[1], original.at<cv::Vec3b>(r, c + 1)[2]) * 2
                    + getLuminosity(original.at<cv::Vec3b>(r - 1, c + 1)[0], original.at<cv::Vec3b>(r - 1, c + 1)[1], original.at<cv::Vec3b>(r - 1, c + 1)[2])
                    + getLuminosity(original.at<cv::Vec3b>(r, c - 1)[0], original.at<cv::Vec3b>(r, c - 1)[1], original.at<cv::Vec3b>(r, c - 1)[2]) * -2
                    + getLuminosity(original.at<cv::Vec3b>(r + 1, c - 1)[0], original.at<cv::Vec3b>(r + 1, c - 1)[1], original.at<cv::Vec3b>(r + 1, c - 1)[2]) * -1
                    + getLuminosity(original.at<cv::Vec3b>(r - 1, c - 1)[0], original.at<cv::Vec3b>(r - 1, c - 1)[1], original.at<cv::Vec3b>(r - 1, c - 1)[2]) * -1;


                // Horizontal
                double horzX;
                horzX = getLuminosity(original.at<cv::Vec3b>(r + 1, c - 1)[0], original.at<cv::Vec3b>(r + 1, c - 1)[1], original.at<cv::Vec3b>(r + 1, c - 1)[2]) * -1
                    + getLuminosity(original.at<cv::Vec3b>(r + 1, c)[0], original.at<cv::Vec3b>(r + 1, c)[1], original.at<cv::Vec3b>(r + 1, c)[2]) * -2
                    + getLuminosity(original.at<cv::Vec3b>(r + 1, c + 1)[0], original.at<cv::Vec3b>(r + 1, c + 1)[1], original.at<cv::Vec3b>(r + 1, c + 1)[2]) * -1
                    + getLuminosity(original.at<cv::Vec3b>(r - 1, c)[0], original.at<cv::Vec3b>(r - 1, c)[1], original.at<cv::Vec3b>(r - 1, c)[2]) * 2
                    + getLuminosity(original.at<cv::Vec3b>(r - 1, c - 1)[0], original.at<cv::Vec3b>(r - 1, c - 1)[1], original.at<cv::Vec3b>(r - 1, c - 1)[2])
                    + getLuminosity(original.at<cv::Vec3b>(r - 1, c + 1)[0], original.at<cv::Vec3b>(r - 1, c + 1)[1], original.at<cv::Vec3b>(r + 1, c - 1)[2]);


                // Magnitude
                double mag = sqrt(pow(vertY, 2.0) + pow(horzX, 2.0));

                // Assign values to each pixel according to magnitude
                if (mag > 100) {
                    modified.at<cv::Vec3b>(r, c)[0] = 255;
                    modified.at<cv::Vec3b>(r, c)[1] = 255;
                    modified.at<cv::Vec3b>(r, c)[2] = 255;
                }

                else if (mag > 30) {
                    modified.at<cv::Vec3b>(r, c)[0] = mag;
                    modified.at<cv::Vec3b>(r, c)[1] = mag;
                    modified.at<cv::Vec3b>(r, c)[2] = mag;
                }
                else {
                    modified.at<cv::Vec3b>(r, c)[0] = 0;
                    modified.at<cv::Vec3b>(r, c)[1] = 0;
                    modified.at<cv::Vec3b>(r, c)[2] = 0;
                }
            }
        }
    }
}


/* Helper function for strobel outline (gets brightness of a pixel */
double getLuminosity(double b, double g, double r) {
	return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}


void approximate() {
    int randomR, randomC;
    double upperGrad, lowerGrad, leftGrad, rightGrad;
    float originalBlue, originalGreen, originalRed;
    double smallest, secondSmallest;
    int pt1Row, pt1Col, pt2Row, pt2Col;
    int count = 0;
    int strength;

    // Blank white canvas
    for (int r = 0; r < original.rows; ++r) {
        for (int c = 0; c < original.cols; ++c) {
            modified.at<cv::Vec3b >(r, c)[0] = 255;
            modified.at<cv::Vec3b >(r, c)[1] = 255;
            modified.at<cv::Vec3b >(r, c)[2] = 255;
        }
    }
    
    do {
        pt1Row = 0; // Initialize all points
        pt1Col = 0;
        pt2Row = 0;
        pt2Col = 0;

        if (count < 2500)
            strength = 90;
        else if (count < 5000)
            strength = 60;
        else if (count < 6000)
            strength = 45;
        else if (count < 8000)
            strength = 30;
        else
            strength = 20;

        // Pick random pixel, this will act as one vertice of the triangle
        randomR = rand() % original.rows;
        randomC = rand() % original.cols;
        originalBlue = original.at<cv::Vec3b>(randomR, randomC)[0];
        originalGreen = original.at<cv::Vec3b>(randomR, randomC)[1];
        originalRed = original.at<cv::Vec3b>(randomR, randomC)[2];


        // Find gradient of pixel directly above randomly chosen pixel.
        if (randomR - 2 < 0) {
            upperGrad = 442; // Highest gradient possible (Does not choose the upper direction)
        }
        else {
            upperGrad = getDistance(originalBlue, originalGreen, originalRed,
                original.at<cv::Vec3b>(randomR - 1, randomC)[0],
                original.at<cv::Vec3b>(randomR - 1, randomC)[1],
                original.at<cv::Vec3b>(randomR - 1, randomC)[2]);
        }

        // Find gradient of pixel directly below randomly chosen pixel.
        if (randomR + 2 > original.rows) {
            lowerGrad = 442;
        }
        else {
            lowerGrad = getDistance(originalBlue, originalGreen, originalRed,
                original.at<cv::Vec3b>(randomR + 1, randomC)[0],
                original.at<cv::Vec3b>(randomR + 1, randomC)[1],
                original.at<cv::Vec3b>(randomR + 1, randomC)[2]);
        }

        // Find gradient of pixel directly right of the randomly chosen pixel.
        if (randomC + 2 > original.cols) {
            rightGrad = 442;
        }
        else {
            rightGrad = getDistance(originalBlue, originalGreen, originalRed,
                original.at<cv::Vec3b>(randomR, randomC + 1)[0],
                original.at<cv::Vec3b>(randomR, randomC + 1)[1],
                original.at<cv::Vec3b>(randomR, randomC + 1)[2]);
        }

        // Find gradient of pixel directly left of the randomly chosen pixel
        if (randomC - 2 < 0) {
            leftGrad = 442;
        }
        else {
            leftGrad = getDistance(originalBlue, originalGreen, originalRed,
                original.at<cv::Vec3b>(randomR, randomC - 1)[0],
                original.at<cv::Vec3b>(randomR, randomC - 1)[1],
                original.at<cv::Vec3b>(randomR, randomC - 1)[2]);
        }

        

        // Now we will compare and get the two smallest gradients
        smallest = std::min(std::min(upperGrad, lowerGrad), std::min(rightGrad, leftGrad));
        // 1 = Up, 2 = Right, 3 = Down, 4 = Left
        if (smallest == upperGrad) {
            smallest = 1;
            secondSmallest = std::min(lowerGrad, std::min(rightGrad, leftGrad));
            if (secondSmallest == lowerGrad)
                secondSmallest = 3;
            else if (secondSmallest == rightGrad)
                secondSmallest = 2;
            else
                secondSmallest = 4;
        }
        else if (smallest == rightGrad) {
            smallest = 2;
            secondSmallest = std::min(std::min(upperGrad, lowerGrad), leftGrad);
            if (secondSmallest == upperGrad)
                secondSmallest = 1;
            else if (secondSmallest == lowerGrad)
                secondSmallest = 3;
            else
                secondSmallest = 4;
        }
        else if (smallest == lowerGrad) {
            smallest = 3;
            secondSmallest = std::min(upperGrad, std::min(rightGrad, leftGrad));
            if (secondSmallest == upperGrad)
                secondSmallest = 1;
            else if (secondSmallest == rightGrad)
                secondSmallest = 2;
            else if (secondSmallest == leftGrad)
                secondSmallest = 4;
        }
        else {
            smallest = 4;
            secondSmallest = std::min(std::min(upperGrad, lowerGrad), rightGrad);
            if (secondSmallest == upperGrad)
                secondSmallest = 1;
            else if (secondSmallest == rightGrad)
                secondSmallest = 2;
            else if (secondSmallest == lowerGrad)
                secondSmallest = 3;
        }
        
        // Now, we will go in each direction of the smallest and secondSmallest gradients,
        // and will stop at a point when the difference in average color is too large.
        // Resulting in two points.
        if (smallest == 1 || secondSmallest == 1) { // Go up.
            int r = randomR - 1;
            int close = 1;
            while (close && r > 1) {
                close = isClose(originalBlue, originalGreen, originalRed,
                    original.at<cv::Vec3b>(r, randomC)[0],
                    original.at<cv::Vec3b>(r, randomC)[1],
                    original.at<cv::Vec3b>(r, randomC)[2], strength);
                --r;
            }
            ++r; // Need to reset r either back to zero, or to the coordinate which isClose failed.
            if (smallest == 1) {
                pt1Row = r;
                pt1Col = randomC;
            }
            else {
                pt2Row = r;
                pt2Col = randomC;
            }
        }
        if (smallest == 2 || secondSmallest == 2) { // Go right
            int c = randomC + 1;
            int close = 1;
            while (close && c + 1 < original.cols) {
                close = isClose(originalBlue, originalGreen, originalRed,
                    original.at<cv::Vec3b>(randomR, c)[0],
                    original.at<cv::Vec3b>(randomR, c)[1],
                    original.at<cv::Vec3b>(randomR, c)[2], strength);
                ++c;
            }
            --c;
            if (smallest == 2) {
                pt1Row = randomR;
                pt1Col = c;
            }
            else {
                pt2Row = randomR;
                pt2Col = c;
            }
        }
        if (smallest == 3 || secondSmallest == 3) { // Go down
            int r = randomR + 1;
            int close = 1;
            while (close && r + 1 < original.rows) {
                close = isClose(originalBlue, originalGreen, originalRed,
                    original.at<cv::Vec3b>(r, randomC)[0],
                    original.at<cv::Vec3b>(r, randomC)[1],
                    original.at<cv::Vec3b>(r, randomC)[2], strength);
                ++r;
            }
            --r;
            if (smallest == 3) {
                pt1Row = r;
                pt1Col = randomC;
            }
            else {
                pt2Row = r;
                pt2Col = randomC;
            }
        }
        if (smallest == 4 || secondSmallest == 4) {
            int c = randomC - 1;
            int close = 1;
            while (close && c > 1) { // Go left
                close = isClose(originalBlue, originalGreen, originalRed,
                    original.at<cv::Vec3b>(randomR, c)[0],
                    original.at<cv::Vec3b>(randomR, c)[1],
                    original.at<cv::Vec3b>(randomR, c)[2], strength);
                --c;
            }
            ++c;
            if (smallest == 4) {
                pt1Row = randomR;
                pt1Col = c;
            }
            else {
                pt2Row = randomR;
                pt2Col = c;
            }
        }
        
        // Fill the triangle with the given points (randomR, randomC), (pt1Row, pt1Col), (pt2Row, pt2Col)
        // 1st, check for base cases (straight lines)
        if ((smallest == 1 && secondSmallest == 3) || (smallest == 3 && secondSmallest == 1)) { // straight line up and down
            // start from top
            
            int r = std::min(pt1Row, pt2Row);
            int stop = std::max(pt1Row, pt2Row);
            while (r <= stop) {
                modified.at<cv::Vec3b>(r, randomC)[0] = originalBlue;
                modified.at<cv::Vec3b>(r, randomC)[1] = originalGreen;
                modified.at<cv::Vec3b>(r, randomC)[2] = originalRed;
                ++r;
            }
        }
        else if ((smallest == 2 && secondSmallest == 4) || (smallest == 4 && secondSmallest == 2)) { // straight line left and right
            // Start from left
            
            int c = std::min(pt1Col, pt2Col);
            int stop = std::max(pt1Col, pt2Col);
            while (c <= stop) {
                modified.at<cv::Vec3b>(randomR, c)[0] = originalBlue;
                modified.at<cv::Vec3b>(randomR, c)[1] = originalGreen;
                modified.at<cv::Vec3b>(randomR, c)[2] = originalRed;
                ++c;
            }		
        }
        else {
            // It's a right triangle.
            int horzDistance, vertDistance;
            int subPerLine;
            int eachLine;

            horzDistance = abs(pt2Col - randomC);
            if (horzDistance == 0)
                horzDistance = abs(pt1Col - randomC);
            vertDistance = abs(pt2Row - randomR);
            if (vertDistance == 0)
                vertDistance = abs(pt1Row - randomR);

            if (horzDistance == 0)
                horzDistance = 1;

            subPerLine = vertDistance / horzDistance;
            eachLine = horzDistance;

            if ((smallest == 2 && secondSmallest == 1) || (smallest == 1 && secondSmallest == 2)) {
                // Draw triangle in 1st quadrant
                for (int r = randomR; r >= randomR - vertDistance; --r) {
                    for (int c = randomC; c <= (randomC + eachLine); ++c) {
                        if (isClose(originalBlue, originalGreen, originalRed,
                            original.at<cv::Vec3b>(r, c)[0],
                            original.at<cv::Vec3b>(r, c)[1],
                            original.at<cv::Vec3b>(r, c)[2], strength)) {
                                modified.at<cv::Vec3b>(r, c)[0] = originalBlue;
                                modified.at<cv::Vec3b>(r, c)[1] = originalGreen;
                                modified.at<cv::Vec3b>(r, c)[2] = originalRed;
                        }
                    }
                    eachLine -= subPerLine;
                }	
                
            }
            else if ((smallest == 2 && secondSmallest == 3) || (smallest == 3 && secondSmallest == 2)) {
                // Draw triangle in 4th quadrant
                for (int r = randomR; r <= randomR + vertDistance; ++r) {
                    for (int c = randomC; c <= (randomC + eachLine); ++c) {
                        if (isClose(originalBlue, originalGreen, originalRed,
                            original.at<cv::Vec3b>(r, c)[0],
                            original.at<cv::Vec3b>(r, c)[1],
                            original.at<cv::Vec3b>(r, c)[2], strength)) {
                            modified.at<cv::Vec3b>(r, c)[0] = originalBlue;
                            modified.at<cv::Vec3b>(r, c)[1] = originalGreen;
                            modified.at<cv::Vec3b>(r, c)[2] = originalRed;
                        }
                    }
                    eachLine -= subPerLine;
                }
            }
            else if ((smallest == 4 && secondSmallest == 3) || (smallest == 3 && secondSmallest == 4)) {
                // Draw triangle in 3rd quadrant
                for (int r = randomR; r <= randomR + vertDistance; ++r) {
                    for (int c = randomC; c >= (randomC - eachLine); --c) {
                        if (isClose(originalBlue, originalGreen, originalRed,
                            original.at<cv::Vec3b>(r, c)[0],
                            original.at<cv::Vec3b>(r, c)[1],
                            original.at<cv::Vec3b>(r, c)[2], strength)) {
                            modified.at<cv::Vec3b>(r, c)[0] = originalBlue;
                            modified.at<cv::Vec3b>(r, c)[1] = originalGreen;
                            modified.at<cv::Vec3b>(r, c)[2] = originalRed;
                        }
                    }
                    eachLine -= subPerLine;
                }
            }
            else {
                // Draw triangle in 2nd quadrant
                for (int r = randomR; r >= randomR - vertDistance; --r) {
                    for (int c = randomC; c >= (randomC - eachLine); --c) {
                        if (isClose(originalBlue, originalGreen, originalRed,
                            original.at<cv::Vec3b>(r, c)[0],
                            original.at<cv::Vec3b>(	r, c)[1],
                            original.at<cv::Vec3b>(r, c)[2], strength)) {
                            modified.at<cv::Vec3b>(r, c)[0] = originalBlue;
                            modified.at<cv::Vec3b>(r, c)[1] = originalGreen;
                            modified.at<cv::Vec3b>(r, c)[2] = originalRed;
                        }
                    }
                    eachLine -= subPerLine;
                }
            }
        }
        // Show the modified image
        cv::imshow("Modified", modified);
        //cv::waitKey(2);
        ++count;
    } while (count < 10000 && cv::waitKey(2) != 27);
}


void motionDetection() {
    if (prevFrame.empty()) {
       original.copyTo(prevFrame); 
    }

    // Compare each pixel of original frame to prevFrame
    for (int r = 0; r < original.rows; r++) {
        for (int c = 0; c < original.cols; c++) {
            float bDiff = std::abs(original.at<cv::Vec3b>(r, c)[0] - prevFrame.at<cv::Vec3b>(r, c)[0]);
            float gDiff = std::abs(original.at<cv::Vec3b>(r, c)[1] - prevFrame.at<cv::Vec3b>(r, c)[1]);
            float rDiff = std::abs(original.at<cv::Vec3b>(r, c)[2] - prevFrame.at<cv::Vec3b>(r, c)[2]);            
            double total = (double)bDiff + (double)gDiff + (double)rDiff;
            if (total > 110) {
                modified.at<cv::Vec3b>(r, c)[0] = 255; 
                modified.at<cv::Vec3b>(r, c)[1] = 255; 
                modified.at<cv::Vec3b>(r, c)[2] = 255; 
            } else {
                modified.at<cv::Vec3b>(r, c)[0] = 0; 
                modified.at<cv::Vec3b>(r, c)[1] = 0; 
                modified.at<cv::Vec3b>(r, c)[2] = 0; 
            }
        }
    }

    // copy original to prevFrame
    for (int r = 0; r < original.rows; r++) {
        for(int c = 0; c < original.cols; c++) {
            prevFrame.at<cv::Vec3b>(r, c)[0] = original.at<cv::Vec3b>(r, c)[0];
            prevFrame.at<cv::Vec3b>(r, c)[1] = original.at<cv::Vec3b>(r, c)[1];
            prevFrame.at<cv::Vec3b>(r, c)[2] = original.at<cv::Vec3b>(r, c)[2];
        }
    }
}


int isClose(float originalB, float originalG, float originalR, float b, float g, float r, int strength) {
	double distance = sqrt(pow(originalB - b, 2) + pow(originalG - g, 2) + pow(originalR - r, 2));
	if (distance < strength) return 1;
	else return 0;
}


double getDistance(float originalBlue, float originalGreen, float originalRed, float blue, float green, float red) {
	return sqrt(pow(originalBlue - blue, 2) + pow(originalGreen - green, 2) + pow(originalRed - red, 2));
}


int smallest(double up, double right, double down, double left) {
	double min = std::min(std::min(down, up), std::min(left, right));
	if (min == up) return 1;
	else if (min = right) return 2;
	else if (min == down) return 3;
	else return 4;
}
