Make it executable:

cd STM32MP157-DK2


chmod +x tests/test.sh
chmod +x tests/simulation/test_simulator.sh
chmod +x tests/integration/test_gpio_hw.sh

Then:

./tests/test.sh

or individual testing:

./tests/test.sh unit
./tests/test.sh simulation
./tests/test.sh integration
./tests/test.sh gpio-info
Overall test flow
                    tests/test.sh
                         |
              +----------+----------+
              |          |          |
              v          v          v
           UNIT       SIMULATION  INTEGRATION
              |          |          |
              v          v          v
       test_gpio.c   simulator     STM32MP157-DK2
                         |          real GPIO
                         v             |
                   Virtual GPIO        |
                         |             |
                         +------+------+
                                |
                                v
                         TEST SUMMARY
                                |
                    +-----------+-----------+
                    |           |           |
                  PASS        FAIL       SKIP

For your STM32MP157-DK2 project, this gives you a clean separation between C unit testing, Python GPIO simulation, and actual board GPIO validation.
