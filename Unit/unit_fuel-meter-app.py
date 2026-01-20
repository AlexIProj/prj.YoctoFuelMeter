import unittest
import ctypes
import os
from datetime import datetime

lib_path = os.path.abspath("libcalcs.so")
fuel_meter_lib = ctypes.CDLL(lib_path)

fuel_meter_lib.CalculateFuelConsumption.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
fuel_meter_lib.CalculateFuelConsumption.restype = ctypes.c_double

fuel_meter_lib.GetMetricConstant.argtypes = []
fuel_meter_lib.GetMetricConstant.restype = ctypes.c_double

fuel_meter_lib.GetImperialConstant.argtypes = []
fuel_meter_lib.GetImperialConstant.restype = ctypes.c_double

METRIC_TO_IMPERIAL = 6.09202687


class TestFuelMeterApp(unittest.TestCase):

    def test_zero_speed_returns_zero(self):
        result = fuel_meter_lib.CalculateFuelConsumption(0, 100, 1)
        self.assertEqual(result, 0.0)

    def test_max_consumption_capped(self):
        result = fuel_meter_lib.CalculateFuelConsumption(10, 1000, 1)
        self.assertEqual(result, 99.9)

    def test_metric_calculation(self):
        # 155 speed pulses, 100 flow pulses -> ~20 L/100km
        result = fuel_meter_lib.CalculateFuelConsumption(155, 100, 1)
        self.assertAlmostEqual(result, 20.0, delta=0.5)

    def test_imperial_calculation(self):
        result = fuel_meter_lib.CalculateFuelConsumption(155, 100, 0)
        expected = 20.0 / METRIC_TO_IMPERIAL  # ~3.28 G/100mi
        self.assertAlmostEqual(result, expected, delta=0.1)

    def test_metric_imperial_consistency(self):
        test_cases = [(155, 100), (100, 50), (200, 100), (500, 100)]

        for speed, flow in test_cases:
            metric = fuel_meter_lib.CalculateFuelConsumption(speed, flow, 1)
            imperial = fuel_meter_lib.CalculateFuelConsumption(speed, flow, 0)
            expected_imperial = metric / METRIC_TO_IMPERIAL
            self.assertAlmostEqual(imperial, expected_imperial, delta=0.01,
                msg=f"Inconsistent at ({speed}, {flow})")

    def test_constants_ratio(self):
        metric_k = fuel_meter_lib.GetMetricConstant()
        imperial_k = fuel_meter_lib.GetImperialConstant()
        ratio = metric_k / imperial_k
        self.assertAlmostEqual(ratio, METRIC_TO_IMPERIAL, delta=0.001)


if __name__ == "__main__":
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    report_filename = f"TestReport_{timestamp}.txt"

    print(f"Test running ... Test Report: {report_filename}\n")

    with open(report_filename, "w") as report_file:
        report_file.write(f"Test Report - {timestamp}\n")
        report_file.write("Fuel Meter Application Unit Tests\n\n")

        runner = unittest.TextTestRunner(stream=report_file, verbosity=2)
        suite = unittest.TestLoader().loadTestsFromTestCase(TestFuelMeterApp)
        result = runner.run(suite)

    if result.wasSuccessful():
        print("All tests passed successfully.")
    else:
        print("Some tests failed. Please check the report for details.")
