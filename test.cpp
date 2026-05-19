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

bool solve(const std::string &input_file) {
    std::string output_file =
        input_file.substr(0, input_file.size() - 3) + ".out";

    std::ifstream f(input_file);
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

    std::string result = lexicographic_paths(grid, mx, mn, K);

    std::ifstream out(output_file);
    std::string expected;
    out >> expected;

    bool correct = result == expected;
    std::cout << input_file << ": " << (correct ? "OK" : "FAIL")
              << " (got: " << result << ", expected: " << expected << ")\n";
    return correct;
}

int main() {
    bool all_ok = true;
    all_ok &= solve("./test/spust01.in");
    all_ok &= solve("./test/spust02.in");
    all_ok &= solve("./test/spust03.in");
    all_ok &= solve("./test/spust04.in");
    all_ok &= solve("./test/spust05.in");
    all_ok &= solve("./test/spust06.in");
    std::cout << (all_ok ? "All tests passed!" : "Some tests failed.") << "\n";
    return all_ok ? 0 : 1;
}
