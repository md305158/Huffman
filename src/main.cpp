#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

#include "../inc/consts.h"
#include "../inc/huffman_encoder.h"
#include "../inc/huffman_tree.h"
#include "../inc/ui.h"

static const console_ui console_ui;

static const std::string mode_compress = "compress";
static const std::string mode_decompress = "decompress";

enum class mode
{
    INVALID = 0,
    COMPRESS,
    DECOMPRESS,
};

/**
 * @brief Klasa reprezentująca opcje wiersza linii poleceń
 */
class option
{
  private:
    const std::string short_name_, long_name_;
    const std::string description_;

    const std::function<void(int &)> function_;

  public:
    /**
     * @brief Tworzy nowy obiekt opcji
     *
     * @param short_name - krótka nazwa
     * @param long_name - długa nazwa
     * @param description - opis
     * @param function - akcja która zostanie wywołana kiedy przełącznik
     * zostanie wykryty
     */
    option(std::string short_name, std::string long_name,
           std::string description, std::function<void(int &)> function)
        : short_name_(std::move(short_name)), long_name_(std::move(long_name)),
          description_(std::move(description)), function_(std::move(function))
    {
    }

    const std::string &get_short_name() const { return this->short_name_; }
    const std::string &get_long_name() const { return this->long_name_; }
    const std::string &get_description() const { return this->description_; }
    const std::function<void(int &)> &get_function() const
    {
        return this->function_;
    }

    /**
     * @brief Funkcja sprawdzająca czy candidate jest równy short_name lub
     * long_name
     *
     * @param candidate - kandydat
     * @return true - kandydat odpowiada short_name lub long_name
     * @return false - jeżeli kandydat nie odpowiada short_name i long_name
     */
    bool matches(const std::string &candidate) const
    {
        return this->get_long_name() == candidate ||
               this->get_short_name() == candidate;
    }
};

static void invalid_usage(const std::string &program_name)
{
    std::stringstream ss;
    ss << "Invalid arguments. Run: " << program_name << " --help for help";
    console_ui.app_error(ss.str());
}

int main(int argc, char *argv[])
{
    try
    {
        const std::string program_name = argv[0];
        std::string input_file, output_file;
        auto mode = mode::INVALID;

        std::vector<option> options{
            option("-h", "--help", "Prints help",
                   [program_name, &options](int &i)
                   {
                       UNUSED(i);
                       std::stringstream ss;

                       ss << "Running: " << program_name << " [OPTIONS]";
                       console_ui.write_message(ss.str());
                       // https://stackoverflow.com/a/20792
                       ss.str(std::string());

                       console_ui.write_message("Options: ");

                       for (const auto &option : options)
                       {
                           ss << "\t" << option.get_short_name() << ", "
                              << option.get_long_name() << " - "
                              << option.get_description();
                           console_ui.write_message(ss.str());
                           ss.str(std::string());
                       }
                       exit(EXIT_SUCCESS);
                   }),
            option("-i", "--input-file", "Input file path [required]",
                   [argc, argv, &input_file](int &i)
                   {
                       if (i + 1 >= argc)
                           console_ui.app_error("Input file not specified");
                       input_file = std::string(argv[i + 1]);
                       i++;
                   }),
            option(
                "-o", "--output-file",
                "Output file path [optional, defaults to $(input-file).out)]",
                [argc, argv, &output_file](int &i)
                {
                    if (i + 1 >= argc)
                        console_ui.app_error("Output file not specified");
                    output_file = std::string(argv[i + 1]);
                    i++;
                }),
            option("-m", "--mode",
                   "Compression algorithm mode <" + mode_compress + "|" +
                       mode_decompress + "> [required]",
                   [argc, argv, &mode](int &i)
                   {
                       if (i + 1 >= argc)
                           console_ui.app_error("Mode not specified");
                       if (argv[i + 1] == mode_compress)
                           mode = mode::COMPRESS;
                       else if (argv[i + 1] == mode_decompress)
                           mode = mode::DECOMPRESS;
                       i++;
                   })};

        if (argc < 2)
            invalid_usage(program_name);

        for (int i = 1; i < argc; i++)
            for (const auto &option : options)
                if (option.matches(argv[i]))
                    option.get_function()(i);

        if (mode == mode::INVALID)
            invalid_usage(program_name);

        if (input_file.empty())
            invalid_usage(program_name);

        if (output_file.empty())
            output_file = input_file + ".out";

        auto encoder = huffman_encoder(input_file, output_file, console_ui);

        switch (mode)
        {
        case mode::COMPRESS:
            encoder.compress_file();
            break;
        case mode::DECOMPRESS:
            encoder.decompress_file();
            break;

        case mode::INVALID:
        default:
            console_ui.app_error("Unhandled mode.");
            break;
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what();
        return EXIT_FAILURE;
    }
}
