#include "mime_type.h"

bool hasExtension(
    const std::string &filePath,
    const std::string &extension)
{

    if (filePath.length() < extension.length())
    {

        return false;
    }

    return filePath.compare(
               filePath.length() - extension.length(),
               extension.length(),
               extension) == 0;
}

std::string MimeType::getMimeType(
    const std::string &filePath)
{

    if (hasExtension(filePath, ".html"))
    {

        return "text/html";
    }

    if (hasExtension(filePath, ".css"))
    {

        return "text/css";
    }

    if (hasExtension(filePath, ".js"))
    {

        return "application/javascript";
    }

    if (hasExtension(filePath, ".png"))
    {

        return "image/png";
    }

    if (hasExtension(filePath, ".jpg") || hasExtension(filePath, ".jpeg"))
    {

        return "image/jpeg";
    }

    return "text/plain";
}

bool MimeType::isBinaryFile(
    const std::string &filePath)
{

    return

        hasExtension(filePath, ".png")

        ||

        hasExtension(filePath, ".jpg")

        ||

        hasExtension(filePath, ".jpeg");
}