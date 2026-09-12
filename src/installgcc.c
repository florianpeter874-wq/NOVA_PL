#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

static int command_exists(const char *command)
{
    char test_command[512];
    int result;

#ifdef _WIN32
    snprintf(
        test_command,
        sizeof(test_command),
        "where %s >nul 2>&1",
        command
    );
#else
    snprintf(
        test_command,
        sizeof(test_command),
        "command -v %s > /dev/null 2>&1",
        command
    );
#endif

    result = system(test_command);

    return result == 0;
}

static int gcc_works(void)
{
    int result;

#ifdef _WIN32
    result = system("gcc --version >nul 2>&1");
#else
    result = system("gcc --version > /dev/null 2>&1");
#endif

    return result == 0;
}

#ifdef _WIN32

static int directory_exists(const char *path)
{
    DWORD attributes;

    attributes = GetFileAttributesA(path);

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }

    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

static int find_ms2_root(char *output, size_t output_size)
{
    const char *paths[] = {
        "C:\\msys64",
        "C:\\msys2",
        "D:\\msys64",
        "D:\\msys2",
        "C:\\Program Files\\msys64",
        NULL
    };

    int i;

    for (i = 0; paths[i] != NULL; i++) {
        if (directory_exists(paths[i])) {
            strncpy(
                output,
                paths[i],
                output_size - 1
            );

            output[output_size - 1] = '\0';

            return 1;
        }
    }

    return 0;
}

static int install_mingw_gcc(const char *msys_root)
{
    char command[1024];
    int result;

    printf(
        "NOVA: MSYS2 found at:\n"
        "      %s\n\n",
        msys_root
    );

    printf(
        "NOVA: installing MinGW-w64 GCC...\n"
    );

    snprintf(
        command,
        sizeof(command),
        "\"%s\\usr\\bin\\bash.exe\" -lc \"pacman --noconfirm -Syu mingw-w64-x86_64-gcc\"",
        msys_root
    );

    result = system(command);

    if (result != 0) {
        fprintf(
            stderr,
            "NOVA: MSYS2 GCC installation failed.\n"
        );

        return 1;
    }

    printf(
        "NOVA: GCC installation command completed.\n"
    );

    return 0;
}

#endif

int main(void)
{
    printf(
        "Nova 1.1\n"
        "the children of the Nova project.\n"
        "Built by Netfloor Software Corporation\n\n"
    );
    printf(
        "=================================\n"
        "       NOVA GCC INSTALLER\n"
        "=================================\n\n"
    );

#ifdef _WIN32

    printf(
        "NOVA: Windows detected.\n"
    );

#else

    printf(
        "NOVA: This installer currently targets Windows.\n"
    );

    return 1;

#endif

    printf(
        "NOVA: checking GCC...\n"
    );

    if (command_exists("gcc") && gcc_works()) {
        printf(
            "\n"
            "NOVA: GCC is already installed.\n\n"
        );

        system("gcc --version");

        printf(
            "\nNOVA: nothing to install.\n"
        );

        return 0;
    }

    printf(
        "NOVA: GCC was not found.\n"
    );

    {
        char msys_root[512];

        if (!find_ms2_root(
                msys_root,
                sizeof(msys_root))) {

            fprintf(
                stderr,
                "\n"
                "NOVA: MSYS2 was not found.\n\n"
                "Install MSYS2 first, then run this installer again.\n"
            );

            return 1;
        }

        if (install_mingw_gcc(msys_root) != 0) {
            return 1;
        }
    }

    printf(
        "\n"
        "NOVA: checking GCC again...\n"
    );

    if (command_exists("gcc") && gcc_works()) {
        printf(
            "\n"
            "NOVA: GCC is ready!\n"
        );

        system("gcc --version");

        return 0;
    }

    printf(
        "\n"
        "NOVA: GCC was installed, but it is not yet available\n"
        "      in the current PATH.\n\n"
        "Restart the terminal and run NOVA again.\n"
    );

    return 0;
}
