
import numpy as np
import control as ct

class GeneralOrderFilter:
    def __init__(self, numerator, denominator, Ts):
        """
        Initialize the filter with numerator and denominator coefficients.
        :param numerator: Array of numerator coefficients [B0, B1, ...]
        :param denominator: Array of denominator coefficients [1, A1, A2, ...]
        :param Ts: Sampling time
        """
        self.Ts = Ts

        # Convert continuous-time transfer function to discrete-time
        Gs = ct.tf(np.array(numerator), np.array(denominator))
        Gz = ct.c2d(Gs, Ts, 'tustin')

        # Extract discrete-time numerator and denominator coefficients
        self.numerator = np.array(Gz.num[0][0])  # Flatten numerator list of arrays
        self.denominator = np.array(Gz.den[0][0])  # Flatten denominator list of arrays

        # Initialize the input and output histories
        self.x = np.zeros(len(self.numerator))  # x(k), x(k-1), ...
        self.y = np.zeros(len(self.denominator))  # y(k), y(k-1), ...

    def compute(self, x_k):
        """
        Compute the filter output for the given input sample.
        :param x_k: Current input sample
        :return: Filtered output y(k)
        """
        self.x[0] = x_k

        # Compute y(k) = sum(Bn*x(k-n)) - sum(An*y(k-n)), where A0 is assumed to be 1
        y_k = np.dot(self.numerator, self.x) - np.dot(self.denominator[1:], self.y[:-1])

        # Update histories for next computation
        self.x[1:] = self.x[:-1]
        self.y[1:] = self.y[:-1]
        self.y[0] = y_k

        return y_k