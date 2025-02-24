
#include "stdafx.h"
#include "common.h"
#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <fstream>
#include <vector>

using namespace std;
using std::vector;

struct HuffmanNode{
    int frequency;
    int value;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(int freq, uchar val) : frequency(freq), value(val), left(nullptr), right(nullptr) {}
};

struct CompareHuffmanNodes{
    bool operator()(const HuffmanNode* a, const HuffmanNode* b) const{
        return a->frequency > b->frequency;
    }
};

void assignBinaryCodes(HuffmanNode* node, string code, vector<string>& codes)
{
    if (node == nullptr)
    {
        return;
    }
    if (node->left == nullptr && node->right == nullptr)
    {
        codes[node->value] = code;
        //node->code = code;
    }
    assignBinaryCodes(node->left, code + "0", codes);
    assignBinaryCodes(node->right, code + "1", codes);
}

HuffmanNode* buildHuffmanTree(vector<int> frequencyVector) {
    //Create a priority queue and populate it with the the nodes created with the frequnecies
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompareHuffmanNodes> priorityQueue;
    for (int i = 0; i < frequencyVector.size(); i++){
        if (frequencyVector[i] > 0) {
            priorityQueue.push(new HuffmanNode(frequencyVector[i], i));
            //cout << "Node: " << i << " - " << frequencyVector[i] << endl;
        }
    }
    while (priorityQueue.size() > 1)
    {
        //Pop the two nodes with the smallest frequency
        HuffmanNode* left = priorityQueue.top();
        priorityQueue.pop();
        HuffmanNode* right = priorityQueue.top();
        priorityQueue.pop();

        //Create a new node with a frequency equal to the sum of the frequencies of the two nodes
        HuffmanNode* parent = new HuffmanNode(left->frequency + right->frequency, 0);
        parent->left = left;
        parent->right = right;

        //Add the new node back to the priority queue
        priorityQueue.push(parent);
    }

    //The last remaining node in the priority queue is the root of the Huffman tree
    HuffmanNode* root = priorityQueue.top();
    priorityQueue.pop();

    return root;
}

vector<int> createFrequencyVector(vector<int> pixel_data) {

    vector<int> frequencyVector(256);

    for (int i = 0; i < pixel_data.size() - 1; i++) {
        frequencyVector[pixel_data[i]] ++;
    }

    /*for (int i = 0; i < 256; i++) {
        cout << i << " - " << frequencyVector[i] << endl;
    }*/

    return frequencyVector;
}

vector<int> createPixelVector(Mat_<uchar> img) {

    int index = 0;
    int num_pixels = img.rows * img.cols;
    vector<int> pixel_data(num_pixels);

    for (int i = 0; i < img.rows; i++){
        for (int j = 0; j < img.cols; j++){
            pixel_data[index] = img(i, j);
            //cout << pixel_data[index] << " ";
            index++;
        }
    }

    return pixel_data;
}

char bitsToChar(std::vector<bool> bits){

    //Create a byte from the bool vector
    char byte = 0;
    for (int i = 0; i < 8; i++)
    {
        if (bits[i])
        {
            byte |= (1 << (7 - i));
        }
    }
    return byte;
}

void writeDataToBinaryFile(const string& filename, vector<int> pixel_data, vector<string> codes, int rows, int cols, Mat_<uchar> img) {
    //Open and clean the file
    ofstream outfile(filename, ios::out, ios::binary);
    outfile.close();
    outfile.open(filename, ios::out | ios::binary | ios::trunc);

    //Check if its open
    if (!outfile.is_open()) {
        cout << "Failed to open file!\n";
        return;
    }

    outfile.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
    outfile.write(reinterpret_cast<const char*>(&cols), sizeof(cols));

    for (int i = 0; i < codes.size(); i++) {

        string code = codes[i];
        int index = i;
        unsigned char sizeChar = static_cast<unsigned char>(code.length());
        unsigned char ucharValue = static_cast<unsigned char>(index);
        outfile.write(reinterpret_cast<const char*>(&index), sizeof(unsigned char));
        outfile.write(reinterpret_cast<const char*>(&sizeChar), sizeof(unsigned char));

        // Write the code bytes
        unsigned char byte = 0;
        int bitCount = 0;

        for (char bit : code) {
            byte <<= 1;
            if (bit == '1') {
                byte |= 1;
            }
            bitCount++;

            if (bitCount == 8) {
                outfile.write(reinterpret_cast<const char*>(&byte), sizeof(unsigned char));
                byte = 0;
                bitCount = 0;
            }
        }

        // Write the remaining bits (if any) as a byte
        if (bitCount > 0) {
            byte <<= (8 - bitCount);
            outfile.write(reinterpret_cast<const char*>(&byte), sizeof(unsigned char));
        }
    }

    int bitCounter = 0;
    uchar bitBuffer = 0;

    for (int i = 0; i < img.rows; i++) {    
        for (int j = 0; j < img.cols; j++) {
            string codification = codes[img(i, j)];
            for (char bit : codification) {
                bitBuffer <<= 1; // Shift the bits in the byte to the left by one position

                if (bit == '1') {
                    bitBuffer |= 1; // Set the rightmost bit to 1 if the current bit is '1'
                }

                bitCounter++;

                if (bitCounter == 8) {
                    outfile.write(reinterpret_cast<const char*>(&bitBuffer), sizeof(unsigned char));
                    bitBuffer = 0;
                    bitCounter = 0;
                }
            }
        }
    }
    if (bitCounter < 8) {
        while (bitCounter < 8) {
            bitBuffer <<= 1; // Shift the bits in the buffer to the left by one position
            bitCounter++;
        }
        outfile.write(reinterpret_cast<const char*>(&bitBuffer), sizeof(unsigned char));
    }
    //Close the file
    outfile.close();    
}

string bytesToBinaryString(const unsigned char codeBytes[], int length) {
    string binaryString;
    for (int i = 0; i < length; i++) {
        unsigned char byte = codeBytes[i];
        for (int j = 7; j >= 0; j--) {
            binaryString += ((byte >> j) & 1) ? "1" : "0";
        }
    }
    return binaryString;
}

int searchInVector(const vector<string>& vec, const string& searchString) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i] == searchString) {
            return i;
        }
    }
    return -1;  // Return -1 if the string is not found
}

void readHeaderDataFromBinaryFile(const string& filename, vector<string>& codes, int& rows, int& cols) {
    ifstream infile(filename, ios::in | ios::binary);

    if (!infile.is_open()) {
        cout << "Failed to open file!\n";
        return;
    }
    //the header of the data
    infile.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    infile.read(reinterpret_cast<char*>(&cols), sizeof(cols));

    for (int i = 0; i < codes.size(); i++) {
        unsigned char ucharValue;
        unsigned char sizeChar;
        string code;

        infile.read(reinterpret_cast<char*>(&ucharValue), 1);
        infile.read(reinterpret_cast<char*>(&sizeChar), sizeof(unsigned char));

        int index = static_cast<int>(ucharValue);
        int length = static_cast<int>(sizeChar);
        int byteLength = (length % 8 != 0) ? ((length + 7) / 8) : (length / 8);
        unsigned char* codeBytes = new unsigned char[byteLength];

        for (int b = 0; b < byteLength; b++) {
            infile.read(reinterpret_cast<char*>(&codeBytes[b]), sizeof(unsigned char));
        }

        code = bytesToBinaryString(codeBytes, byteLength);

        if (code.length() > length) {
            code = code.substr(0, length);
        }

        codes[index] = code;

        delete[] codeBytes;
    }

    infile.close();
}


void encodeData(Mat_<uchar> img) {
    vector<int> pixelData = createPixelVector(img);
    vector<int> frequencyVector = createFrequencyVector(pixelData);
    HuffmanNode* root = buildHuffmanTree(frequencyVector);
    const string& filename = "output.bin";
    vector<string> codes(256);

    assignBinaryCodes(root, "", codes);

    writeDataToBinaryFile(filename, pixelData, codes, img.rows, img.cols, img);

    vector<string> decompressedCodes(256);
    int rows, cols;
    readHeaderDataFromBinaryFile(filename, decompressedCodes, rows, cols);

    for (int i = 0; i < 256; i++) {
        cout << "Pixel " << i << " | Code: " << decompressedCodes[i] << endl;
    }

    cout << rows << " " << cols;
}

int main()
{
	Mat_<uchar> img = imread("Images/camera.bmp", IMREAD_GRAYSCALE);

    // Check if the image was successfully loaded
    if (img.empty())
    {
        cout << "Failed to load image." << endl;
        return -1;
    }

    encodeData(img);

	return 0;
}