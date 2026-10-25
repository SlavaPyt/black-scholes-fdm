#include "bs.hpp"

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

// Everything the user can set from the command line, with defaults.
struct CliOptions {
    BSParams params{100.0, 100.0, 0.0, 0.2, 1.0};  // S, K, r, sigma, T
    std::string style = "call";                    // "call" or "put"
    std::string method = "bs";                     // pricing method; only "bs" for now

    bool grid_mode = false;  // price over a grid of spot prices instead of a single point
    double grid_start = 50.0;
    double grid_end = 150.0;
    int grid_n = 101;
    std::string out_path;  // CSV output, required in grid mode

    bool show_help = false;
};

void print_usage(std::ostream& os, const std::string& prog) {
    os << "Usage:\n"
       << "  " << prog << " [options]\n"
       << "  " << prog << " --grid-S START END N --out FILE [options]\n\n"
       << "Model parameters:\n"
       << "  --S VALUE        spot price of the underlying   (default 100)\n"
       << "  --K VALUE        strike price                   (default 100)\n"
       << "  --r VALUE        risk-free rate, per year       (default 0)\n"
       << "  --sigma VALUE    volatility, per year           (default 0.2)\n"
       << "  --T VALUE        time to maturity, in years     (default 1)\n\n"
       << "Options:\n"
       << "  --style call|put option type                    (default call)\n"
       << "  --method bs      pricing method                 (default bs)\n"
       << "  --grid-S START END N\n"
       << "                   price on N evenly spaced spot prices and write a CSV\n"
       << "  --out FILE       CSV output path (required with --grid-S)\n"
       << "  -h, --help       show this message\n";
}

// Parses the whole string as a double, e.g. "1.5" is fine but "1.5abc" is rejected.
double parse_double(const std::string& flag, const std::string& text) {
    std::size_t consumed = 0;
    double value = 0.0;
    try {
        value = std::stod(text, &consumed);
    } catch (const std::exception&) {
        consumed = 0;
    }
    if (consumed == 0 || consumed != text.size()) {
        throw std::invalid_argument(flag + ": '" + text + "' is not a number");
    }
    return value;
}

int parse_int(const std::string& flag, const std::string& text) {
    std::size_t consumed = 0;
    int value = 0;
    try {
        value = std::stoi(text, &consumed);
    } catch (const std::exception&) {
        consumed = 0;
    }
    if (consumed == 0 || consumed != text.size()) {
        throw std::invalid_argument(flag + ": '" + text + "' is not an integer");
    }
    return value;
}

// Returns the value that follows a flag and advances the index past it.
std::string take_value(int argc, char* argv[], int& i, const std::string& flag) {
    if (i + 1 >= argc) {
        throw std::invalid_argument(flag + " requires a value");
    }
    return argv[++i];
}

CliOptions parse_args(int argc, char* argv[]) {
    CliOptions opt;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opt.show_help = true;
        } else if (arg == "--S") {
            opt.params.S = parse_double(arg, take_value(argc, argv, i, arg));
        } else if (arg == "--K") {
            opt.params.K = parse_double(arg, take_value(argc, argv, i, arg));
        } else if (arg == "--r") {
            opt.params.r = parse_double(arg, take_value(argc, argv, i, arg));
        } else if (arg == "--sigma") {
            opt.params.sigma = parse_double(arg, take_value(argc, argv, i, arg));
        } else if (arg == "--T") {
            opt.params.T = parse_double(arg, take_value(argc, argv, i, arg));
        } else if (arg == "--style") {
            opt.style = take_value(argc, argv, i, arg);
        } else if (arg == "--method") {
            opt.method = take_value(argc, argv, i, arg);
        } else if (arg == "--out") {
            opt.out_path = take_value(argc, argv, i, arg);
        } else if (arg == "--grid-S") {
            opt.grid_start = parse_double(arg, take_value(argc, argv, i, arg));
            opt.grid_end = parse_double(arg, take_value(argc, argv, i, arg));
            opt.grid_n = parse_int(arg, take_value(argc, argv, i, arg));
            opt.grid_mode = true;
        } else {
            throw std::invalid_argument("unknown argument '" + arg + "'");
        }
    }
    return opt;
}

// Checks that don't belong to the model itself (the model validates S, K, r, sigma, T).
void validate(const CliOptions& opt) {
    if (opt.style != "call" && opt.style != "put") {
        throw std::invalid_argument("--style must be 'call' or 'put', got '" + opt.style + "'");
    }
    if (opt.method != "bs") {
        throw std::invalid_argument("--method '" + opt.method +
                                    "' is not implemented (only 'bs' is available)");
    }
    if (opt.grid_mode) {
        if (opt.grid_n < 2) {
            throw std::invalid_argument("--grid-S needs at least 2 points");
        }
        if (opt.grid_start <= 0.0 || opt.grid_end <= opt.grid_start) {
            throw std::invalid_argument("--grid-S needs 0 < START < END");
        }
        if (opt.out_path.empty()) {
            throw std::invalid_argument("--out is required with --grid-S");
        }
    }
}

void run_single(const CliOptions& opt) {
    const BlackScholes model(opt.params);
    const Quote q = make_option(opt.style, model)->quote();

    std::cout << "price=" << q.price << "\n"
              << "delta=" << q.delta << "\n"
              << "gamma=" << q.gamma << "\n"
              << "vega=" << q.vega << "\n"
              << "theta=" << q.theta << "\n";
}

void run_grid(const CliOptions& opt) {
    std::ofstream file(opt.out_path);
    if (!file) {
        throw std::runtime_error("cannot open '" + opt.out_path + "' for writing");
    }

    file << std::setprecision(10);
    file << "S,price,delta,gamma,vega,theta\n";

    BSParams p = opt.params;
    for (int j = 0; j < opt.grid_n; ++j) {
        // N points means N - 1 intervals; computing from j avoids accumulating rounding error.
        p.S = opt.grid_start + (opt.grid_end - opt.grid_start) * j / (opt.grid_n - 1);

        const Quote q = make_option(opt.style, BlackScholes(p))->quote();
        file << p.S << ',' << q.price << ',' << q.delta << ',' << q.gamma << ',' << q.vega << ','
             << q.theta << '\n';
    }

    if (!file) {
        throw std::runtime_error("failed while writing '" + opt.out_path + "'");
    }
    std::cout << "Wrote " << opt.grid_n << " rows (S from " << opt.grid_start << " to "
              << opt.grid_end << ") to " << opt.out_path << "\n";
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string prog = argc > 0 ? argv[0] : "black_scholes";

    try {
        const CliOptions opt = parse_args(argc, argv);
        if (opt.show_help) {
            print_usage(std::cout, prog);
            return 0;
        }
        validate(opt);

        if (opt.grid_mode) {
            run_grid(opt);
        } else {
            run_single(opt);
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n"
                  << "Run '" << prog << " --help' for usage.\n";
        return 1;
    }
}
