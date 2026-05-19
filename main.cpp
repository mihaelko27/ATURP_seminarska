#include <algorithm>
#include <climits>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

static const std::vector<std::pair<int, int>> deltas = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
static const char dirs[] = "JSVZ";

// ─── 1. YOUR ORIGINAL FUNCTION, UNTOUCHED ───
std::string lexicographic_paths(const std::vector<std::vector<int>> &grid,
                                int start_val, int end_val, int64_t k) {
    int rows = grid.size(), cols = grid[0].size();

    std::vector<std::pair<int, int>> starts;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (grid[r][c] == start_val)
                starts.push_back({r, c});
    std::sort(starts.begin(), starts.end());

    std::vector<std::vector<int64_t>> dp(rows, std::vector<int64_t>(cols, -1));

    std::function<int64_t(int, int, int64_t)> count_paths =
        [&](int r, int c, int64_t budget) -> int64_t {
        if (dp[r][c] != -1)
            return std::min(dp[r][c], budget);
        int64_t total = (grid[r][c] == end_val) ? 1 : 0;
        for (int i = 0; i < 4; i++) {
            if (total >= budget)
                return total;
            int nr = r + deltas[i].first, nc = c + deltas[i].second;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
                grid[r][c] > grid[nr][nc])
                total += count_paths(nr, nc, budget - total);
        }
        dp[r][c] = total;
        return total;
    };

    int64_t count = 0;
    for (std::pair<int, int> start : starts) {
        int sr = start.first, sc = start.second;
        int64_t subtree = count_paths(sr, sc, k - count);
        if (count + subtree < k) {
            count += subtree;
            continue;
        }

        std::string path;
        path.reserve(rows * cols);
        struct Frame {
            int r, c, idx;
        };
        std::vector<Frame> stack;
        stack.reserve(rows * cols);
        stack.push_back({sr, sc, 0});

        while (!stack.empty()) {
            Frame &frame = stack.back();
            int &r = frame.r, &c = frame.c, &idx = frame.idx;

            if (idx == 0 && grid[r][c] == end_val) {
                count++;
                if (count == k)
                    return path;
            }

            bool pushed = false;
            while (idx < 4) {
                int nr = r + deltas[idx].first, nc = c + deltas[idx].second;
                char dir = dirs[idx];
                idx++;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
                    grid[r][c] > grid[nr][nc]) {
                    int64_t sub = count_paths(nr, nc, k - count);
                    if (count + sub < k) {
                        count += sub;
                        continue;
                    }
                    path.push_back(dir);
                    stack.push_back({nr, nc, 0});
                    pushed = true;
                    break;
                }
            }
            if (!pushed) {
                if (!path.empty())
                    path.pop_back();
                stack.pop_back();
            }
        }
    }
    throw std::runtime_error("Only " + std::to_string(count) +
                             " paths exist, k=" + std::to_string(k));
}

// ─── 2. BRUTE FORCE ───
void brute_force_dfs(const std::vector<std::vector<int>> &grid, int r, int c,
                     int end_val, std::string &current,
                     std::vector<std::string> &all_paths) {
    int rows = grid.size(), cols = grid[0].size();
    if (grid[r][c] == end_val)
        all_paths.push_back(current);
    for (int i = 0; i < 4; i++) {
        int nr = r + deltas[i].first, nc = c + deltas[i].second;
        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
            grid[r][c] > grid[nr][nc]) {
            current.push_back(dirs[i]);
            brute_force_dfs(grid, nr, nc, end_val, current, all_paths);
            current.pop_back();
        }
    }
}

std::string brute_force(const std::vector<std::vector<int>> &grid,
                        int start_val, int end_val, int64_t k) {
    int rows = grid.size(), cols = grid[0].size();

    int sr, sc;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (grid[r][c] == start_val) {
                sr = r;
                sc = c;
            }

    std::string cur;
    std::vector<std::string> all_paths;
    brute_force_dfs(grid, sr, sc, end_val, cur, all_paths);
    std::sort(all_paths.begin(), all_paths.end());

    if (k > (int64_t)all_paths.size())
        throw std::runtime_error("Not enough paths");
    return all_paths[k - 1];
}

bool run_tests() {
    std::vector<std::string> test_files = {
        "./test/spust01.in", "./test/spust02.in", "./test/spust03.in",
        "./test/spust04.in", "./test/spust05.in", "./test/spust06.in",
    };

    bool all_ok = true;
    for (const std::string &input_file : test_files) {
        std::string output_file =
            input_file.substr(0, input_file.size() - 2) + "out";

        std::ifstream f(input_file);
        if (!f) {
            std::cerr << "Cannot open: " << input_file << "\n";
            all_ok = false;
            continue;
        }

        int64_t N, M, K;
        f >> N >> M >> K;
        std::vector<std::vector<int>> grid(N, std::vector<int>(M));
        int mn = INT_MAX, mx = INT_MIN;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++) {
                f >> grid[i][j];
                mn = std::min(mn, grid[i][j]);
                mx = std::max(mx, grid[i][j]);
            }

        std::ifstream out(output_file);
        if (!out) {
            std::cerr << "Cannot open: " << output_file << "\n";
            all_ok = false;
            continue;
        }
        std::string expected;
        out >> expected;

        std::string result = lexicographic_paths(grid, mx, mn, K);

        bool ok = result == expected;
        std::cout << input_file << ": " << (ok ? "OK" : "FAIL")
                  << " (got: " << result << ", expected: " << expected << ")\n";
        all_ok &= ok;
    }

    std::cout << (all_ok ? "All tests passed!" : "Some tests failed.") << "\n";
    return all_ok;
}

int main(int argc, char *argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--test") {
        return run_tests() ? 0 : 1;
    }
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>  OR  " << argv[0]
                  << " --test\n";
        return 1;
    }

    std::ifstream f(argv[1]);
    if (!f) {
        std::cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }

    int64_t N, M, K;
    f >> N >> M >> K;
    std::vector<std::vector<int>> grid(N, std::vector<int>(M));
    int mn = INT_MAX, mx = INT_MIN;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++) {
            f >> grid[i][j];
            mn = std::min(mn, grid[i][j]);
            mx = std::max(mx, grid[i][j]);
        }

    // std::string result = lexicographic_paths(grid, mx, mn, K);
    std::string result = brute_force(grid, mx, mn, K);
    std::cout << result << "\n";
    return 0;
}
