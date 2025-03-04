#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>

class BMPImage {
private:
    std::vector<unsigned char> pixelData;
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    int width, height;
    int bytesPerPixel;
    bool isTopDown;
    std::string inputFileName, outputFileName;

public:
    BMPImage(const std::string& filename) : inputFileName(filename) {
        loadBMP();
    }

    void loadBMP() {
        std::ifstream file(inputFileName, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + inputFileName);
        }

        file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

        width = infoHeader.biWidth;
        height = abs(infoHeader.biHeight);
        bytesPerPixel = infoHeader.biBitCount / 8;
        isTopDown = infoHeader.biHeight < 0;

        if (bytesPerPixel != 3 && bytesPerPixel != 4) {
            throw std::runtime_error("Only 24-bit and 32-bit BMP files are supported.");
        }

        int rowSize = (width * bytesPerPixel + 3) & ~3;
        pixelData.resize(rowSize * height);

        file.seekg(fileHeader.bfOffBits, std::ios::beg);
        file.read(reinterpret_cast<char*>(pixelData.data()), pixelData.size());
        file.close();
    }

    bool isWhite(unsigned char r, unsigned char g, unsigned char b) const {
        return (r == 255 && g == 255 && b == 255);
    }

    bool isBlack(unsigned char r, unsigned char g, unsigned char b) const {
        return (r == 0 && g == 0 && b == 0);
    }

    void displayBMP() const {
        int rowSize = (width * bytesPerPixel + 3) & ~3;
        for (int y = (isTopDown ? 0 : height - 1); (isTopDown ? y < height : y >= 0); (isTopDown ? ++y : --y)) {
            for (int x = 0; x < width; ++x) {
                int index = y * rowSize + x * bytesPerPixel;
                unsigned char blue = pixelData[index];
                unsigned char green = pixelData[index + 1];
                unsigned char red = pixelData[index + 2];
                if (isWhite(red, green, blue))
                    std::cout << ' ';
                else if (isBlack(red, green, blue))
                    std::cout << '*';
                else
                    throw std::runtime_error("Invalid color detected. Only black and white images are allowed.");
            }
            std::cout << '\n';
        }
    }

    void drawLine(int x1, int y1, int x2, int y2) {
        int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
        int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
        int err = dx - dy, e2;

        while (true) {
            setPixel(x1, y1, 0, 0, 0);
            if (x1 == x2 && y1 == y2) break;
            e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x1 += sx; }
            if (e2 < dx) { err += dx; y1 += sy; }
        }
    }

    void drawCross() {
        drawLine(0, 0, width - 1, height - 1);
        drawLine(0, height - 1, width - 1, 0);
    }

    void saveBMP(const std::string& filename) {
        outputFileName = filename;
        std::ofstream file(outputFileName, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + outputFileName);
        }

        file.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        file.write(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
        file.write(reinterpret_cast<char*>(pixelData.data()), pixelData.size());
        file.close();
    }

private:
    void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        int rowSize = (width * bytesPerPixel + 3) & ~3;
        int index = y * rowSize + x * bytesPerPixel;
        pixelData[index] = b;
        pixelData[index + 1] = g;
        pixelData[index + 2] = r;
    }
};

int main() {
    try {
        std::string inputFile, outputFile;
        std::cout << ">> Enter input BMP file name: ";
        std::cin >> inputFile;

        BMPImage image(inputFile);
        image.displayBMP();

        image.drawCross();
        std::cout << "Modified Image:\n";
        image.displayBMP();

        std::cout << ">> Enter output BMP file name: ";
        std::cin >> outputFile;
        image.saveBMP(outputFile);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}