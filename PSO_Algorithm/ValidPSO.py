
import math
import numpy as np
import matplotlib.pyplot as plt
import control as ct
import Filter as ft

if __name__ == "__main__":
    # System plant
    Ts = 0.5  # Sampling time
    Runtime = 500  # Simulation runtime

    # Define transfer function coefficients
    p_num_s = np.array([6.8])
    p_den_s = np.array([1, 10.02, 0.2])

    # Continuous-time system
    Gs = ct.tf(p_num_s, p_den_s)

    # Discrete-time filter simulation
    filter_sim = ft.GeneralOrderFilter(p_num_s, p_den_s, Ts)

    # Generate time vector and step input
    Time = np.arange(0, Runtime, Ts)
    R = np.ones(len(Time))  # Step input
    Y_filter = np.zeros(len(Time))

    # Simulate filter response
    for k, t in enumerate(Time):
        Y_filter[k] = filter_sim.compute(R[k])

    # Step response for continuous-time system
    T_step, Y_step = ct.step_response(ct.tf(np.array([15]), np.array([1, 15, 0.44263285])), Time)

    # Plot results
    plt.figure(figsize=(12, 6))
    plt.plot(Time, R, label="Input (Step)", linestyle="--", color="gray")
    plt.plot(Time, Y_filter, label="GeneralOrderFilter Response", linestyle="-", color="blue")
    plt.plot(T_step, Y_step, label="Step Response (Control Library)", linestyle=":", color="red")
    plt.title("Comparison of GeneralOrderFilter and Step Response")
    plt.xlabel("Time (s)")
    plt.ylabel("Amplitude")
    plt.legend()
    plt.grid()
    plt.show()