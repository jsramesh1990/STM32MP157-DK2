typedef enum {
    GPIO_BACKEND_NONE = 0,
    GPIO_BACKEND_SYSFS,
    GPIO_BACKEND_LIBGPIOD
} gpio_backend_t;

typedef struct {
    int chip;
    int line;

    char direction[16];

    int value;

    int initialized;

    int use_libgpiod;

    gpio_backend_t backend;

    gpio_gpiod_context_t gpiod;

} gpio_config_t;
