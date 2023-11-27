#include "catch.hpp"

#include <unsafe/iostream.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

TEST_CASE("stdio")
{
    CHECK(unsafe::streambuf_FILE(std::cin.rdbuf()) == stdin);
    CHECK(unsafe::streambuf_FILE(std::wcin.rdbuf()) == stdin);
    CHECK(unsafe::streambuf_FILE(std::cout.rdbuf()) == stdout);
    CHECK(unsafe::streambuf_FILE(std::wcout.rdbuf()) == stdout);
    CHECK(unsafe::streambuf_FILE(std::cerr.rdbuf()) == stderr);
    CHECK(unsafe::streambuf_FILE(std::wcerr.rdbuf()) == stderr);
    CHECK(unsafe::streambuf_FILE(std::clog.rdbuf()) == stderr);
    CHECK(unsafe::streambuf_FILE(std::wclog.rdbuf()) == stderr);

    CHECK(unsafe::streambuf_fileno(std::cin.rdbuf()) == 0);
    CHECK(unsafe::streambuf_fileno(std::wcin.rdbuf()) == 0);
    CHECK(unsafe::streambuf_fileno(std::cout.rdbuf()) == 1);
    CHECK(unsafe::streambuf_fileno(std::wcout.rdbuf()) == 1);
    CHECK(unsafe::streambuf_fileno(std::cerr.rdbuf()) == 2);
    CHECK(unsafe::streambuf_fileno(std::wcerr.rdbuf()) == 2);
    CHECK(unsafe::streambuf_fileno(std::clog.rdbuf()) == 2);
    CHECK(unsafe::streambuf_fileno(std::wclog.rdbuf()) == 2);

#ifdef _WIN32
    HANDLE hStdIn  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
#else
    int hStdIn = STDIN_FILENO;
    int hStdOut = STDOUT_FILENO;
    int hStdErr = STDERR_FILENO;
#endif

    CHECK(unsafe::streambuf_native_handle(std::cin.rdbuf()) == hStdIn);
    CHECK(unsafe::streambuf_native_handle(std::wcin.rdbuf()) == hStdIn);
    CHECK(unsafe::streambuf_native_handle(std::cout.rdbuf()) == hStdOut);
    CHECK(unsafe::streambuf_native_handle(std::wcout.rdbuf()) == hStdOut);
    CHECK(unsafe::streambuf_native_handle(std::cerr.rdbuf()) == hStdErr);
    CHECK(unsafe::streambuf_native_handle(std::wcerr.rdbuf()) == hStdErr);
    CHECK(unsafe::streambuf_native_handle(std::clog.rdbuf()) == hStdErr);
    CHECK(unsafe::streambuf_native_handle(std::wclog.rdbuf()) == hStdErr);
}

TEST_CASE("fstream")
{
    const char* filename = "unsafe.test.fstream";

    {
        std::ofstream stream(filename, std::ios_base::binary);
        REQUIRE(stream.is_open());

        CHECK(stream.write("0", 1));
        CHECK(stream.flush());

        FILE* file = unsafe::filebuf_FILE(stream.rdbuf());
        CHECK(std::fwrite("12", 1, 2, file) == 2);
        CHECK(std::fflush(file) == 0);

        int fd = unsafe::filebuf_fileno(stream.rdbuf());
#ifdef _WIN32
        CHECK(_write(fd, "345", 3) == 3);
#else
        CHECK(write(fd, "345", 3) == 3);
#endif

        auto hFile = unsafe::filebuf_native_handle(stream.rdbuf());
#ifdef _WIN32
        DWORD n = 0;
        CHECK((WriteFile(hFile, "6789", 4, &n, nullptr) && n == 4));
#else
        CHECK(write(hFile, "6789", 4) == 4);
#endif
    }

    {   // IMPORTANT: Unbuffered read first.
        char buf[20] = {};
        std::ifstream stream(filename, std::ios_base::binary);
        REQUIRE(stream.is_open());

        auto hFile = unsafe::filebuf_native_handle(stream.rdbuf());
#ifdef _WIN32
        DWORD n = 0;
        CHECK((ReadFile(hFile, buf, 1, &n, nullptr) && n == 1));
#else
        CHECK(read(hFile, buf, 1) == 1);
#endif
        CHECK((buf[0] == '0'));

        int fd = unsafe::filebuf_fileno(stream.rdbuf());
#ifdef _WIN32
        CHECK(_read(fd, buf + 1, 2) == 2);
#else
        CHECK(read(fd, buf + 1, 2) == 2);
#endif
        CHECK((buf[1] == '1' && buf[2] == '2'));

        FILE* file = unsafe::filebuf_FILE(stream.rdbuf());
        std::setvbuf(file, nullptr, _IONBF, 0); // Turn off buffering.
        CHECK(std::fread(buf + 3, 1, 3, file) == 3);
        CHECK((buf[3] == '3' && buf[4] == '4' && buf[5] == '5'));
        
        // CHECK(stream.read(buf + 6, 4)); // UCRT has at least 1 byte buffering.
        CHECK(std::fread(buf + 6, 1, 4, file) == 4);
        CHECK((buf[6] == '6' && buf[7] == '7' && buf[8] == '8' && buf[9] == '9'));
    }

    std::remove(filename);
}
