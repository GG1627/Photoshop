
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <stdexcept>
using namespace std;
#pragma pack(1)

struct Header {
    char idLength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;
    short xOrigin;
    short yOrigin;
    short width;
    short height;
    char bitsPerPixel;
    char imageDescriptor;
};

void readFile(const string& fileName, Header& header, vector<unsigned char>& colorData) {
    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << fileName << endl;
        return;
    }
    file.read(reinterpret_cast<char*>(&header), sizeof(Header));
    int dataSize = header.width * header.height * 3;
    colorData.resize(dataSize);
    file.read(reinterpret_cast<char*>(colorData.data()), dataSize);
    file.close();
}

void writeFile(const string& fileName, const Header& header, const vector<unsigned char>& colorData) {
    ofstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cerr << "Failed to create output file: " << fileName << endl;
        return;
    }
    file.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    file.write(reinterpret_cast<const char*>(colorData.data()), colorData.size());
    file.close();
}

vector<unsigned char> multiply(vector<unsigned char>& colorData1, vector<unsigned char>& colorData2) {
    vector<unsigned char> result(colorData1.size());
    for(int i = 0; i < colorData1.size(); i++) {
        float topNormalized = (float)(colorData1[i]) / 255.0f;
        float bottomNormalized = (float)(colorData2[i]) / 255.0f;

        float resultNormalized = topNormalized * bottomNormalized;
        result[i] = (unsigned char)((resultNormalized * 255.0f) + 0.5f);
    }
    return result;
}

vector<unsigned char> subtract(const vector<unsigned char>& colorData1, const vector<unsigned char>& colorData2) {
    vector<unsigned char> result(colorData1.size());
    for(int i = 0; i < colorData1.size(); i++) {
        int difference = colorData1[i] - colorData2[i];
        result[i] = max(0, difference);
    }
    return result;
}

vector<unsigned char> overlay(const vector<unsigned char>& colorData1, const vector<unsigned char>& colorData2) {
    vector<unsigned char> result(colorData1.size());
    for(int i = 0; i < colorData1.size(); i++) {
        // normalize
        float normColor1 = colorData1[i] / 255.0;
        float normColor2 = colorData2[i] / 255.0;

        // overlay
        if (normColor2 <= 0.5) {
            result[i] = (unsigned char)(((2 * normColor1 * normColor2) * 255) + 0.5f);
        } else {
            result[i] = (unsigned char)(((1 - 2 * (1 - normColor1) * (1 - normColor2)) * 255) + 0.5f);
        }
    }
    return result;
}

vector<unsigned char> screen(const vector<unsigned char>& colorData1, const vector<unsigned char>& colorData2) {
    vector<unsigned char> result(colorData1.size());
    for(int i = 0; i < colorData1.size(); i++) {
        float topNormalized = (float)(colorData1[i]) / 255.0f;
        float bottomNormalized = (float)(colorData2[i]) / 255.0f;
        float resultNormalized = 1.0f - ((1.0f - topNormalized) * (1.0f - bottomNormalized));
        result[i] = (unsigned char)((resultNormalized * 255.0f) + 0.5f);
    }
    return result;
}

vector<unsigned char> combine(const vector<unsigned char>& redData, const vector<unsigned char>& greenData, const vector<unsigned char>& blueData) {

    vector<unsigned char> combinedData(redData.size());
    for (int i = 0; i < redData.size(); i += 3) {
        combinedData[i + 2] = redData[i];       // red
        combinedData[i + 1] = greenData[i];    // green
        combinedData[i] = blueData[i];    // blue
    }
    return combinedData;
}


vector<unsigned char> rotate180(const vector<unsigned char>& colorData) {
    vector<unsigned char> flippedData(colorData.size());

    int pixelSize = 3; // Assuming each pixel has RGB channels

    for(int i = colorData.size() - 3; i >= 0; i -= pixelSize) {
        int destIndex = colorData.size() - i - pixelSize;
        flippedData[destIndex] = colorData[i];            // Red channel
        flippedData[destIndex + 1] = colorData[i + 1];    // Green channel
        flippedData[destIndex + 2] = colorData[i + 2];    // Blue channel
    }

    return flippedData;
}

vector<unsigned char> extractRedChannel(vector<unsigned char>& colorData) {
    for(int i = 0; i < colorData.size(); i += 3) {
        colorData[i] = colorData[i+2];
        colorData[i + 1] = colorData[i+2];
    }
    return colorData;
}

vector<unsigned char> extractGreenChannel(vector<unsigned char>& colorData) {
    for(int i = 0; i < colorData.size(); i += 3) {
        colorData[i] = colorData[i+1];
        colorData[i + 2] = colorData[i+1];
    }
    return colorData;
}

vector<unsigned char> extractBlueChannel(vector<unsigned char>& colorData) {
    for(int i = 0; i < colorData.size(); i += 3) {
        colorData[i + 1] = colorData[i];
        colorData[i + 2] = colorData[i];
    }
    return colorData;
}

float clamp(float value) {
    return max(0.0f, min(value, 255.0f));
}

vector<unsigned char> addRed(vector<unsigned char> colorData, int addedValue) {

    for(int i = 0; i < colorData.size(); i += 3) {
        int newValue = colorData[i] + addedValue;
        colorData[i] = clamp(newValue);
    }
    return colorData;
}

vector<unsigned char> addGreen(vector<unsigned char> colorData, int addedValue) {

    for(int i = 1; i < colorData.size(); i += 3) {
        int newValue = colorData[i] + addedValue;
        colorData[i] = clamp(newValue);
    }
    return colorData;
}

vector<unsigned char> addBlue(vector<unsigned char> colorData, int addedValue) {

    for(int i = 2; i < colorData.size(); i += 3) {
        int newValue = colorData[i] + addedValue;
        colorData[i] = clamp(newValue);
    }
    return colorData;
}


vector<unsigned char> scaleRed(vector<unsigned char> colorData, int scaleFactor) {
    // normalize
    for(int i = 0; i < colorData.size(); i += 3) {
        float normalizedRed = (float)(colorData[i + 2]) / 255.0f;
        // scale
        colorData[i + 2] = (unsigned char)(clamp(normalizedRed * scaleFactor * 255.0f));
    }
    return colorData;
}

vector<unsigned char> scaleGreen(vector<unsigned char> colorData, int scaleFactor) {
    // normalize
    for(int i = 0; i < colorData.size(); i += 3) {
        float normalizedGreen = (float)(colorData[i + 1]) / 255.0f;
        // scale
        colorData[i + 1] = (unsigned char)(clamp(normalizedGreen * scaleFactor * 255.0f));
    }
    return colorData;
}

vector<unsigned char> scaleBlue(vector<unsigned char> colorData, int scaleFactor) {
    // normalize
    for(int i = 0; i < colorData.size(); i += 3) {
        float normalizedBlue = (float)(colorData[i]) / 255.0f;
        // scale
        colorData[i] = (unsigned char)(clamp(normalizedBlue * scaleFactor * 255.0f));
    }
    return colorData;
}

bool isValidOutputFileName(const char* filename) {
    if (std::string(filename).length() < 4 || std::string(filename).substr(strlen(filename) - 4) != ".tga") {
        return false;
    }
    return true;
}

bool isValidInputFileName(const char* filename) {
    if (std::string(filename).length() < 4 || std::string(filename).substr(strlen(filename) - 4) != ".tga") {
        return false;
    }
    return true;
}

bool isValidInputFileName2(char* filename) {
    ifstream file(std::string(filename), ios::binary);
    file.open(std::string(filename));
    if(file.is_open()){
        return true;
    }
    else{
        return false;
    }
}


bool isValidCommand(const char* command) {
    vector<string> validCommands = {
            "multiply", "subtract", "overlay", "screen", "combine", "flip","onlyred", "onlygreen", "onlyblue",
            "addred", "addgreen", "addblue", "scalered", "scalegreen", "scaleblue"
    };
    for(int i = 0; i < validCommands.size(); i++){
        if(validCommands[i] == std::string(command)){
            return true;
        }
    }
    return false;
}


////////////////////////////// COMMANDS /////////////////////////////////////
void multiply(const char* outputFilename, const char* firstImageFilename, const char* secondImageFilename) {
    Header layer1Header, pattern2Header;
    vector<unsigned char> layer1ColorData, pattern2ColorData;

    readFile(firstImageFilename, layer1Header, layer1ColorData);
    readFile(secondImageFilename, pattern2Header, pattern2ColorData);
    vector<unsigned char> multiplyBlendedColorData = multiply(layer1ColorData, pattern2ColorData);
    writeFile(outputFilename, layer1Header, multiplyBlendedColorData);
    //cout << "Multiplication successful" << endl;
}
void subtract(const char* outputFilename, const char* inputFilename1, const char* inputFilename2) {
    Header header1, header2;
    vector<unsigned char> colorData1, colorData2;

    readFile(inputFilename1, header1, colorData1);
    readFile(inputFilename2, header2, colorData2);

    vector<unsigned char> subtractedColorData = subtract(colorData1, colorData2);
    writeFile(outputFilename, header1, subtractedColorData);
}

void overlay(const char* outputFilename, const char* inputFilename1, const char* inputFilename2) {
    Header header1, header2;
    vector<unsigned char> colorData1, colorData2;

    readFile(inputFilename1, header1, colorData1);
    readFile(inputFilename2, header2, colorData2);

    vector<unsigned char> overlayBlendedColorData = overlay(colorData1, colorData2);
    writeFile(outputFilename, header1, overlayBlendedColorData);
}

void combine(const char* outputFilename, const char* redFilename, const char* greenFilename, const char* blueFilename) {
    Header redHeader, greenHeader, blueHeader;
    vector<unsigned char> redCD, greenCD, blueCD;

    readFile(redFilename, redHeader, redCD);
    readFile(greenFilename, greenHeader, greenCD);
    readFile(blueFilename, blueHeader, blueCD);

    vector<unsigned char> combinedBlendedColorData = combine(redCD, greenCD, blueCD);
    writeFile(outputFilename, redHeader, combinedBlendedColorData);
}

void flip(const char* outputFilename, const char* inputFilename) {
    Header flippedHeader;
    vector<unsigned char> flippedCD;
    readFile(inputFilename, flippedHeader, flippedCD);
    vector<unsigned char> flippedResults = rotate180(flippedCD);
    writeFile(outputFilename, flippedHeader, flippedResults);
    //cout << "worked" << endl;
}

void onlyred(const char* outputFilename, const char* inputFilename) {
    Header redHeader;
    vector<unsigned char> redCD;
    readFile(inputFilename, redHeader, redCD);
    vector<unsigned char> onlyRedData = extractRedChannel(redCD);
    writeFile(outputFilename, redHeader, onlyRedData);
}

void onlygreen(const char* outputFilename, const char* inputFilename) {
    Header greenHeader;
    vector<unsigned char> greenCD;
    readFile(inputFilename, greenHeader, greenCD);
    vector<unsigned char> onlyGreenData = extractGreenChannel(greenCD);
    writeFile(outputFilename, greenHeader, onlyGreenData);
}
void onlyblue(const char* outputFilename, const char* inputFilename) {
    Header blueHeader;
    vector<unsigned char> blueCD;
    readFile(inputFilename, blueHeader, blueCD);
    vector<unsigned char> onlyBlueData = extractBlueChannel(blueCD);
    writeFile(outputFilename, blueHeader, onlyBlueData);
}

void addred(const char* outputFilename, const char* inputFilename, int valueToAdd) {
    Header redHeader;
    vector<unsigned char> redCD;
    readFile(inputFilename, redHeader, redCD);
    vector<unsigned char> addedRedData = addRed(redCD, valueToAdd);
    writeFile(outputFilename, redHeader, addedRedData);
}

void addgreen(const char* outputFilename, const char* inputFilename, int valueToAdd) {
    Header greenHeader;
    vector<unsigned char> greenCD;
    readFile(inputFilename, greenHeader, greenCD);
    vector<unsigned char> addedGreenData = addGreen(greenCD, valueToAdd);
    writeFile(outputFilename, greenHeader, addedGreenData);
}

void addblue(const char* outputFilename, const char* inputFilename, int valueToAdd) {
    Header blueHeader;
    vector<unsigned char> blueCD;
    readFile(inputFilename, blueHeader, blueCD);
    vector<unsigned char> addedBlueData = addBlue(blueCD, valueToAdd);
    writeFile(outputFilename, blueHeader, addedBlueData);
}

void scalered(const char* outputFilename, const char* inputFilename, int scaleFactor) {
    Header scaledRedHeader;
    vector<unsigned char> scaledRedCD;
    readFile(inputFilename, scaledRedHeader, scaledRedCD);
    vector<unsigned char> scaledRedData = scaleBlue(scaledRedCD, scaleFactor);
    writeFile(outputFilename, scaledRedHeader, scaledRedData);
}

void scalegreen(const char* outputFilename, const char* inputFilename, int scaleFactor) {
    Header scaledGreenHeader;
    vector<unsigned char> scaledGreenCD;
    readFile(inputFilename, scaledGreenHeader, scaledGreenCD);
    vector<unsigned char> scaledGreenData = scaleGreen(scaledGreenCD, scaleFactor);
    writeFile(outputFilename, scaledGreenHeader, scaledGreenData);
}

void scaleblue(const char* outputFilename, const char* inputFilename, int scaleFactor) {
    Header scaledBlueHeader;
    vector<unsigned char> scaledBlueCD;
    readFile(inputFilename, scaledBlueHeader, scaledBlueCD);
    vector<unsigned char> scaledBlueData = scaleRed(scaledBlueCD, scaleFactor);
    writeFile(outputFilename, scaledBlueHeader, scaledBlueData);
}

void screenM(const char* outputFilename, const char* inputFilename1, const char* inputFilename2) {
    Header screenHeader1, screenHeader2;
    vector<unsigned char> screenColorData1, screenColorData2;
    readFile(inputFilename1, screenHeader1, screenColorData1);
    readFile(inputFilename2, screenHeader2, screenColorData2);
    vector<unsigned char> screenBlendedColorData = screen(screenColorData1, screenColorData2);
    writeFile(outputFilename, screenHeader1, screenBlendedColorData);
}





int main(int argc, char* argv[]) {
    // Check for help message or insufficient arguments
    if (argc == 1 || strcmp(argv[1], "--help") == 0) {
        cout << "Project 2: Image Processing, Spring 2024" << endl;
        cout << endl;
        cout << "Usage:" << endl;
        cout << "\t./project2.out [output] [firstImage] [method] [...]" << endl;
        return 0;
    }

    // Validate output file name
    if (!isValidOutputFileName(argv[1])) {
        cout << "Invalid file name." << endl;
        return 1;
    }

    // Validate input file name
    if (!isValidInputFileName(argv[2])) {
        cout << "Invalid file name." << endl;
        return 1;
    }

    // Validate existence of input file
    if (!isValidInputFileName2(argv[2])) {
        cout << "File does not exist." << endl;
        return 1;
    }

    const char* trackingImage = argv[2]; // Initial source image

    int i = 3; // Start from the first method argument
    while (i < argc) {
//         //Validate method name
//        if (!isValidCommand(argv[i])) {
//            cout << "Invalid method name." << endl;
//            return 1;
//        }

        // Process additional arguments based on the method
        if (strcmp(argv[i], "multiply") == 0) {
            if (argv[i+1] == nullptr) {
                cout << "Missing argument." << endl;
                return 1;

            }

            else if (argv[i+1] != nullptr){
                if (!isValidInputFileName2(argv[i+1])) {
                    cout << "Invalid argument, invalid file name." << endl;
                    return 1;
                }
                else if (!isValidCommand(argv[i])) {
                    cout << "Invalid method name." << endl;
                    return 1;
                }
                else{
                    multiply(argv[1], trackingImage, argv[i+1]);
                    cout << "worked ju" << endl;
                }
            }
        }

        else if (strcmp(argv[i], "subtract") == 0) {
            if (argv[i + 1] == nullptr) {
                cout << "Missing argument." << endl;
                return 1;
            }
            else if (argv[i + 1] != nullptr) {
                if (!isValidInputFileName2(argv[i + 1])) {
                    cout << "Invalid argument, invalid file name." << endl;
                    return 1;
                }
                else if(!isValidCommand(argv[i])){
                    cout << "Invalid method name." << endl;
                    return 1;
                }
                else {
                    subtract(argv[1], trackingImage, argv[i+1]);
                    cout << "worked" << endl;
                    trackingImage = argv[1];
                }
            }
        }

        else if (strcmp(argv[i], "overlay") == 0) {
            if (argv[i+1] == nullptr) {
                cout << "Missing argument." << endl;
                return 1;

            }

            else if (argv[i+1] != nullptr){
                if (!isValidInputFileName2(argv[i+1])) {
                    cout << "Invalid argument, invalid file name." << endl;
                    return 1;
                }
                else if (!isValidCommand(argv[i])) {
                    cout << "Invalid method name." << endl;
                    return 1;
                }
                else{
                    overlay(argv[1], trackingImage, argv[4]);
                    trackingImage = argv[1];
                    //cout << "worked" << endl;
                }
            }
        }


        else if (strcmp(argv[i], "screen") == 0) {
            if (argv[i+1] == nullptr) {
                cout << "Missing argument." << endl;
                return 1;

            }

            else if (argv[i+1] != nullptr){
                if (!isValidInputFileName2(argv[i+1])) {
                    cout << "Invalid argument, invalid file name." << endl;
                    return 1;
                }
                else if (!isValidCommand(argv[i])) {
                    cout << "Invalid method name." << endl;
                    return 1;
                }
                else{
                    screenM(argv[1], trackingImage, argv[4]);
                    //cout << "worked" << endl;
                    trackingImage = argv[1];
                }
            }
        }


        else if (strcmp(argv[i], "addblue") == 0){
            if(argv[i+1] == nullptr){
                cout << "Missing argument." << endl;
            }
            else{
                try {
                    int value = std::stoi(argv[i + 1]); // Convert argument to integer
                    addred(argv[1], trackingImage, value);
                    trackingImage = argv[1];
                }
                catch (std::invalid_argument &e) {
                    // Handle the case where the argument is not a valid integer
                    cout << "Invalid argument, expected number." << endl;
                    return 1;
                }
            }
        }
        else if (strcmp(argv[i], "addgreen") == 0){
            if(argv[i+1] == nullptr){
                cout << "Missing argument." << endl;
            }
            else{
                try {
                    int value = std::stoi(argv[i + 1]); // Convert argument to integer
                    addgreen(argv[1], trackingImage, value);
                    trackingImage = argv[1];
                }
                catch (std::invalid_argument &e) {
                    // Handle the case where the argument is not a valid integer
                    cout << "Invalid argument, expected number." << endl;
                    return 1;
                }
            }
        }
        else if (strcmp(argv[i], "addred") == 0){
            if(argv[i+1] == nullptr){
                cout << "Missing argument." << endl;
            }
            else{
                try {
                    int value = std::stoi(argv[i + 1]); // Convert argument to integer
                    addblue(argv[1], trackingImage, value);
                    trackingImage = argv[1];
                }
                catch (std::invalid_argument &e) {
                    // Handle the case where the argument is not a valid integer
                    cout << "Invalid argument, expected number." << endl;
                    return 1;
                }
            }
        }

        else if (strcmp(argv[i], "scalered") == 0){
            if(argv[i+1] == nullptr){
                cout << "Missing argument." << endl;
            }
            else{
                try {
                    int value = std::stoi(argv[i + 1]); // Convert argument to integer
                    scaleblue(argv[1], trackingImage, value);
                    cout << "worked" << endl;
                    trackingImage = argv[1];
                }
                catch (std::invalid_argument &e) {
                    // Handle the case where the argument is not a valid integer
                    cout << "Invalid argument, expected number." << endl;
                    return 1;
                }
            }
        }
        else if (strcmp(argv[i], "scalegreen") == 0){
            if(argv[i+1] == nullptr){
                cout << "Missing argument." << endl;
            }
            else{
                try {
                    int value = std::stoi(argv[i + 1]); // Convert argument to integer
                    scalegreen(argv[1], trackingImage, value);
                    trackingImage = argv[1];
                }
                catch (std::invalid_argument &e) {
                    // Handle the case where the argument is not a valid integer
                    cout << "Invalid argument, expected number." << endl;
                    return 1;
                }
            }
        }
        else if (strcmp(argv[i], "scaleblue") == 0){
            if(argv[i+1] == nullptr){
                cout << "Missing argument." << endl;
            }
            else{
                try {
                    int value = std::stoi(argv[i + 1]); // Convert argument to integer
                    scalered(argv[1], trackingImage, value);
                    trackingImage = argv[1];
                }
                catch (std::invalid_argument &e) {
                    // Handle the case where the argument is not a valid integer
                    cout << "Invalid argument, expected number." << endl;
                    return 1;
                }
            }
        }
        else if (strcmp(argv[i], "flip") == 0) {
            flip(argv[1], trackingImage);
            cout << "worked ahhhh" << endl;
            trackingImage = argv[1];

        }
        else if (strcmp(argv[i], "onlyblue") == 0){
            onlyblue(argv[1], trackingImage);
            trackingImage = argv[1];
        }
        else if (strcmp(argv[i], "onlygreen") == 0){
            onlygreen(argv[1], trackingImage);
            trackingImage = argv[1];
        }
        else if (strcmp(argv[i], "onlyred") == 0){
            onlyred(argv[1], trackingImage);
            trackingImage = argv[1];
        }
        else if (strcmp(argv[i], "combine") == 0){
            combine(argv[1], trackingImage, argv[i+1], argv[i+2]);
            trackingImage = argv[1];
        }

        // Move to the next method argument
        i++;
    }

    return 0;
}
