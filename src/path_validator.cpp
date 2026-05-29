#include "path_validator.h"

bool PathValidator::isSafePath(
        const std::string& path
) {

    if(path.find("..")
       != std::string::npos) {

        return false;
    }

    return true;
}