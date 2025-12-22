/**
 * @file SDCardManager.cpp
 * @brief Реализация управления SD картой
 */

#include "Storage/SDCardManager.hpp"

// Внешние объявления
extern bool sd_initialized;

/**
 * @brief Инициализация SD карты
 */
bool setupSDCard()
{
    if (!SD_MMC.begin("/sdcard", true))
    {
        Serial.println("SD Card Mount Failed");
        sd_initialized = false;
        return false;
    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        sd_initialized = false;
        return false;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
        Serial.println("MMC");
    else if (cardType == CARD_SD)
        Serial.println("SDSC");
    else if (cardType == CARD_SDHC)
        Serial.println("SDHC");
    else
        Serial.println("UNKNOWN");

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    sd_initialized = true;
    Serial.println("SD Card initialized successfully");
    return true;
}

/**
 * @brief Сохранение фотографии и метаданных на SD карту
 */
bool savePhotoToSD(const char *filename, camera_fb_t *fb, const DynamicJsonDocument &doc)
{
    if (!sd_initialized || !fb || fb->format != PIXFORMAT_JPEG)
    {
        Serial.println("Invalid parameters for photo saving");
        return false;
    }

    String pathPhoto = (String)filename + ".jpg";
    Serial.printf("Saving photo: %s, size: %zu bytes\n", pathPhoto.c_str(), fb->len);

    File filePhoto = SD_MMC.open(pathPhoto.c_str(), FILE_WRITE);
    if (!filePhoto)
    {
        Serial.println("Failed to open file for writing");
        return false;
    }

    size_t written = filePhoto.write(fb->buf, fb->len);
    filePhoto.close();

    if (written != fb->len)
    {
        Serial.printf("Write failed: %zu of %zu bytes written\n", written, fb->len);
        return false;
    }

    if (!verifyFile(pathPhoto, fb->len))
        return false;

    Serial.printf("Photo saved successfully: %s\n", pathPhoto.c_str());

    String pathData = (String)filename + ".json";
    Serial.printf("Saving data: %s\n", pathData.c_str());

    File fileData = SD_MMC.open(pathData.c_str(), FILE_WRITE);
    if (!fileData)
    {
        Serial.println("Failed to open file for writing");
        return false;
    }

    bool success = serializeJson(doc, fileData) != 0;
    fileData.close();

    if (success)
        Serial.println("Metadata saved successfully");
    else
        Serial.println("Failed to write metadata");

    return success;
}

/**
 * @brief Верификация сохраненного файла
 */
bool verifyFile(const String &path, size_t expectedSize)
{
    File file = SD_MMC.open(path.c_str(), FILE_READ);
    if (!file)
    {
        Serial.println("Cannot verify saved file");
        return false;
    }

    size_t fileSize = file.size();
    file.close();

    if (fileSize != expectedSize)
    {
        Serial.printf("File size mismatch: %zu vs %zu\n", fileSize, expectedSize);
        return false;
    }

    return true;
}

/**
 * @brief Получение списка файлов
 */
String listFiles()
{
    if (!sd_initialized)
        return "SD Card not available";

    String fileList = "<html><head><title>Saved Photos</title></head><body>";
    fileList += "<h2>Saved Photos:</h2><ul>";
    File root = SD_MMC.open("/");
    
    if (!root)
        return "Failed to open SD root";
    
    File file = root.openNextFile();
    int count = 0;
    
    while(file && count < 20)
    {
        if(!file.isDirectory() && String(file.name()).endsWith(".jpg"))
        {
            fileList += "<li>" + String(file.name()) + " (" + String(file.size()) + " bytes)</li>";
            count++;
        }
        file.close();
        file = root.openNextFile();
    }
    
    fileList += "</ul>";
    fileList += "<p><a href='/'>Back to main page</a></p>";
    fileList += "</body></html>";
    root.close();
    
    return fileList;
}