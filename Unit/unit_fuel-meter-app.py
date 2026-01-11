import unittest
import ctypes
import os
import sys
from datetime import datetime

lib_path = os.path.abspath("libcalcs.so")
fuel_meter_lib = ctypes.CDLL(lib_path)

fuel_meter_lib.CalculateFuelConsumption.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
fuel_meter_lib.CalculateFuelConsumption.restype = ctypes.c_double

fuel_meter_lib.GetMetricConstant.argtypes = []
fuel_meter_lib.GetMetricConstant.restype = ctypes.c_double

fuel_meter_lib.GetImperialConstant.argtypes = []
fuel_meter_lib.GetImperialConstant.restype = ctypes.c_double

class TestFuelMeterApp(unittest.TestCase):
    def setUp(self):
        self.k_metric = fuel_meter_lib.GetMetricConstant()
        self.k_imperial = fuel_meter_lib.GetImperialConstant()

        self.tolerance = 0.001

    def test_idle_zero_speed(self):
        speed_hz = 0
        flow_hz = 20

        actual_val= fuel_meter_lib.CalculateFuelConsumption(speed_hz, flow_hz, 1)

        print(f"[IDLE ZERO SPEED] Input({speed_hz}, {flow_hz}) -> Metric C: {actual_val:.4f}")

        self.assertEqual(actual_val, 0.0)

    def test_max_saturation(self):
        speed_hz = 10
        flow_hz = 1000

        actual_val= fuel_meter_lib.CalculateFuelConsumption(speed_hz, flow_hz, 1)

        print(f"[MAX SATURATION] Input({speed_hz}, {flow_hz}) -> Metric C: {actual_val:.4f}")
        self.assertAlmostEqual(actual_val, 99.9, 0.0)

    def test_imperial_steady_state(self):
        # Speed = 60 Mi/h -> 100Hz
        # , Flow = 5 Gal/h -> 63Hz
        # Fuel Consumption = (5 / 60) * 100 = 8.3333
        speed_hz = 100
        flow_hz = 63

        actual_val = fuel_meter_lib.CalculateFuelConsumption(speed_hz, flow_hz, 0)
        expected_val = (flow_hz / speed_hz) * self.k_imperial

        print(f"[IMPERIAL] Input({speed_hz}, {flow_hz}) -> C: {actual_val:.4f} vs Py: {expected_val:.4f} (K={self.k_imperial:.2f})")

        self.assertAlmostEqual(actual_val, expected_val, delta=self.tolerance)

    def test_metric_steady_state(self):
        # Speed = 100 Km/h -> 
        # Flow = 9.9 L /h
        # Fuel Consumption = (9.9 / 100) * 100 = 9.9 L/100Km
        speed_hz = 100
        flow_hz = 33

        actual_val = fuel_meter_lib.CalculateFuelConsumption(speed_hz, flow_hz, 1)
        expected_val = (flow_hz / speed_hz) * self.k_metric

        print(f"[METRIC] Input({speed_hz}, {flow_hz}) -> C: {actual_val:.4f} vs Py: {expected_val:.4f} (K={self.k_metric:.2f})")
        self.assertAlmostEqual(actual_val, expected_val, delta=self.tolerance)

    def test_dynamic_metric_scenario(self):
        print("\n[METRIC DYNAMIC SCENARIO]")


if __name__ == "__main__":
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    script_dir = os.path.dirname(os.path.abspath(__file__))

    report_filename = f"TestReport_{timestamp}.txt"
    full_report_path = os.path.join(script_dir, report_filename)

    print(f"Test running ... Test Report: {report_filename}\n")

    with open  (report_filename, "w") as report_file:
        report_file.write(f"Test Report - {timestamp}\n")
        report_file.write("Fuel Meter Application Unit Tests\n\n")

        runner = unittest.TextTestRunner(stream=report_file, verbosity=2)
        suite = unittest.TestLoader().loadTestsFromTestCase(TestFuelMeterApp)
        result = runner.run(suite)

    if result.wasSuccessful():
        print("All tests passed successfully.")
    else:
        print("Some tests failed. Please check the report for details.")