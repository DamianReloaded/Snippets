#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>

// Activation Function Definitions
namespace activation {
    inline double relu(double x) {
        return std::max(0.0, x);
    }

    inline double leaky_relu(double x, double alpha = 0.01) {
        return (x > 0) ? x : alpha * x;
    }
    
    inline double log_activation(double x, double epsilon = 1e-4) {
        return std::log(std::max(epsilon, x));  // Avoid log(0) or negative values
    }

    inline double relu_prime(double x) {
        return x > 0 ? 1.0 : 0.0;
    }

    inline double log_prime(double x, double epsilon = 1e-4) {
        return 1.0 / std::max(epsilon, x); // Gradient of log(x) is 1/x
    }
}

// Neural Network Class
class NeuralNetwork {
public:
    NeuralNetwork(const std::vector<int>& layers, bool use_log_activation = false, double learning_rate = 0.01)
        : layers(layers), use_log_activation(use_log_activation), learning_rate(learning_rate) {
        initialize_weights();
    }

void train(const std::vector<std::vector<double>>& X, const std::vector<std::vector<double>>& y, int max_epochs = 5000, double min_loss_threshold = 1e-4, int patience = 100) {
    double prev_loss = std::numeric_limits<double>::infinity(); // Start with a very large loss value
    int epochs_since_improvement = 0;  // Track how many epochs since the last improvement

    for (int epoch = 0; epoch < max_epochs; ++epoch) {
        // Forward pass
        forward(X);

        // Compute loss (Mean Squared Error)
        double loss = compute_loss(y);
        if (epoch % 100 == 0) {
            std::cout << "Epoch " << epoch << " - Loss: " << loss << std::endl;
        }

        // Check for early stopping based on loss improvement
        if (std::abs(prev_loss - loss) < min_loss_threshold) {
            epochs_since_improvement++;
        } else {
            epochs_since_improvement = 0; // Reset if there is an improvement
        }

        if (epochs_since_improvement >= patience) {
            std::cout << "Early stopping: Loss has not improved for " << patience << " epochs." << std::endl;
            break;
        }

        // Backward pass (Gradient Descent)
        backward(y);

        // Update weights
        update_weights();

        prev_loss = loss; // Update previous loss for next comparison
    }
}


    // Forward pass function to compute outputs for given input X
    void forward(const std::vector<std::vector<double>>& X) {
        for (size_t i = 0; i < X.size(); ++i) {
            std::vector<double> input = X[i];
            activations.clear(); // reset activations for each forward pass

            // Loop over layers
            for (size_t layer = 0; layer < layers.size() - 1; ++layer) {
                std::vector<double> output(layers[layer + 1], 0.0);
                for (int j = 0; j < layers[layer + 1]; ++j) {
                    double net_input = biases[layer][j];
                    for (int k = 0; k < layers[layer]; ++k) {
                        net_input += weights[layer][k][j] * input[k];
                    }

                    if (use_log_activation) {
                        output[j] = activation::log_activation(net_input);
                    } else {
                        output[j] = activation::leaky_relu(net_input);
                    }
                }
                activations.push_back(output);
                input = output;
            }

            // Save final output layer
            final_output = activations.back();
        }
    }

    // Compute the loss (Mean Squared Error)
    double compute_loss(const std::vector<std::vector<double>>& y) {
        double loss = 0.0;
        for (size_t i = 0; i < y.size(); ++i) {
            for (size_t j = 0; j < y[i].size(); ++j) {
                loss += std::pow(final_output[j] - y[i][j], 2);
            }
        }
        return loss / y.size();
    }

    // Backward pass to compute gradients
    void backward(const std::vector<std::vector<double>>& y) {
        // Calculate output layer gradient
        gradients.clear();
        std::vector<double> output_gradients(layers.back());
        for (size_t i = 0; i < y.size(); ++i) {
            for (size_t j = 0; j < y[i].size(); ++j) {
                output_gradients[j] = 2 * (final_output[j] - y[i][j]); // MSE derivative
            }
        }
        gradients.push_back(output_gradients);

        // Backpropagate gradients
        for (int layer = layers.size() - 2; layer >= 0; --layer) {
            std::vector<double> layer_gradients(layers[layer], 0.0);

            // Update gradient for previous layer
            for (int j = 0; j < layers[layer]; ++j) {
                for (int k = 0; k < layers[layer + 1]; ++k) {
                    double derivative = use_log_activation ? activation::log_prime(activations[layer][j]) : activation::relu_prime(activations[layer][j]);
                    layer_gradients[j] += weights[layer][j][k] * gradients.back()[k] * derivative;
                }
            }

            gradients.push_back(layer_gradients);
        }
    }

    // Update weights based on gradients
    void update_weights() {
        for (int layer = 0; layer < layers.size() - 1; ++layer) {
            for (int i = 0; i < layers[layer]; ++i) {
                for (int j = 0; j < layers[layer + 1]; ++j) {
                    weights[layer][i][j] -= learning_rate * gradients[layer][i] * activations[layer][j];
                }
            }

            // Update biases
            for (int j = 0; j < layers[layer + 1]; ++j) {
                biases[layer][j] -= learning_rate * gradients[layer][j];
            }
        }
    }

    // Get final output of the network (for testing)
    const std::vector<double>& get_final_output() const {
        return final_output;
    }

private:
    std::vector<int> layers;
    bool use_log_activation;
    double learning_rate;

    std::vector<std::vector<std::vector<double>>> weights;
    std::vector<std::vector<double>> biases;
    std::vector<std::vector<double>> activations;
    std::vector<double> final_output;
    std::vector<std::vector<double>> gradients;

    // Initialize weights and biases
void initialize_weights() {
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0.0, 1.0);

    weights.resize(layers.size() - 1);
    biases.resize(layers.size() - 1);

    for (size_t i = 0; i < layers.size() - 1; ++i) {
        weights[i].resize(layers[i], std::vector<double>(layers[i + 1], 0.0));
        biases[i].resize(layers[i + 1], 0.0);

        // Initialize weights using Xavier/Glorot initialization
        double stddev = std::sqrt(2.0 / (layers[i] + layers[i + 1]));
        for (int j = 0; j < layers[i]; ++j) {
            for (int k = 0; k < layers[i + 1]; ++k) {
                weights[i][j][k] = distribution(generator) * stddev;
            }
        }
    }
}

int main() {
    // Example: XOR problem with 2 hidden layers
    std::vector<int> layers = {2, 4, 4, 1}; // 2 input, 2 hidden layers with 4 neurons, 1 output
    NeuralNetwork nn_relu(layers, false); // ReLU network
    NeuralNetwork nn_log(layers, true);  // Log network

    // Training data for XOR (simple example)
    std::vector<std::vector<double>> X = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    std::vector<std::vector<double>> y = {{0}, {1}, {1}, {0}}; // XOR targets

    // Train networks for 5000 epochs and compare
    auto start = std::chrono::high_resolution_clock::now();
    nn_relu.train(X, y, 5000);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_relu = end - start;

    start = std::chrono::high_resolution_clock::now();
    nn_log.train(X, y, 5000);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_log = end - start;

    std::cout << "\nReLU training time: " << duration_relu.count() << " seconds\n";
    std::cout << "Log-based activation training time: " << duration_log.count() << " seconds\n";

    // Print final outputs for both networks
    std::cout << "\nReLU Final Predictions:\n";
    const auto& relu_output = nn_relu.get_final_output();
    for (size_t i = 0; i < X.size(); ++i) {
        std::cout << "x = (" << X[i][0] << ", " << X[i][1] << "), Predicted = " << relu_output[i] << ", True = " << y[i][0] << std::endl;
    }

    std::cout << "\nLog-based Final Predictions:\n";
    const auto& log_output = nn_log.get_final_output();
    for (size_t i = 0; i < X.size(); ++i) {
        std::cout << "x = (" << X[i][0] << ", " << X[i][1] << "), Predicted = " << log_output[i] << ", True = " << y[i][0] << std::endl;
    }

    return 0;
}
