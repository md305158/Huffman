#pragma once
#include <string>

/**
 * @brief Prosty interfejs służący do komunikacji z urzytkownikiem
 */
class ui
{
  public:
    virtual ~ui() = default;
    /**
     * @brief Wyświetla komunikat użytkownikowi
     * @param msg - komunikat
     */
    virtual void write_message(const std::string &msg) const = 0;

    /**
     * @brief Obsługuje błąd aplikacji
     * @param error_msg - komunikat błędu
     */
    virtual void app_error(const std::string &error_msg) const = 0;
};

/**
 * @brief Prosta implementacja konsolowego interfejsu użytkownika. Pisze na
 * standardowe wyjście i wyjście błędu
 */
class console_ui final : public ui
{
  public:
    /**
     * @brief Wyświetla komunikat na stdout
     * @param msg - komunikat do wyświetlenia
     */
    void write_message(const std::string &msg) const override;

    /**
     * @brief Wypisuje błąd na stderr i kończy działanie programu z kodem
     * EXIT_FAILURE
     * @param error_msg - komunikat błędu
     */
    void app_error(const std::string &error_msg) const override;
};
