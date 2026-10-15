# Прерывать выполнение при ошибках
set -e

# Список папок с тестами, которые нужно проверить
TESTS=(
    "test/test_blink"
    "test/test_gpio_interrupts"
    "test/test_i2c_wheels"
    "test/test_separate_uart"
    "test/test_stdio_mirror"
)

echo "=== Запуск проверки сборки тестов ==="

# Очищаем старую сборку перед началом
make clean BUILD_DIR=build_test

for TEST_PATH in "${TESTS[@]}"; do
    echo "--------------------------------------------------"
    echo "Сборка: $TEST_PATH..."
    echo "--------------------------------------------------"
    
    # Конфигурируем и собираем конкретный тест через ваш Makefile
    if make build BUILD_DIR=build_test top="$TEST_PATH"; then
        echo "Успешно собран: $TEST_PATH"
    else
        echo "Ошибка сборки: $TEST_PATH"
        exit 1
    fi
done

echo "=================================================="
echo "Все тесты успешно скомпилировались!"
echo "=================================================="
