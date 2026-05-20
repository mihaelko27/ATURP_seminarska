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

struct PathCounter {
    const std::vector<std::vector<int>> &grid;
    std::vector<std::vector<int64_t>> &dp;
    int rows, cols, end_val;

    int64_t count(int r, int c, int64_t budget) {
        if (dp[r][c] != -1)
            return std::min(dp[r][c], budget);
        int64_t total = (grid[r][c] == end_val) ? 1 : 0;
        for (int i = 0; i < 4; i++) {
            if (total >= budget)
                return total;
            int nr = r + deltas[i].first, nc = c + deltas[i].second;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
                grid[r][c] > grid[nr][nc])
                total += count(nr, nc, budget - total);
        }
        dp[r][c] = total;
        return total;
    }

    bool trace(int r, int c, int64_t k, int64_t &counted, std::string &path) {
        if (grid[r][c] == end_val) {
            counted++;
            if (counted == k)
                return true;
        }
        for (int i = 0; i < 4; i++) {
            int nr = r + deltas[i].first, nc = c + deltas[i].second;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
                grid[r][c] > grid[nr][nc]) {
                int64_t sub = count(nr, nc, k - counted);
                if (counted + sub < k) {
                    counted += sub;
                    continue;
                }
                path.push_back(dirs[i]);
                if (trace(nr, nc, k, counted, path))
                    return true;
            }
        }
        return false;
    }
};

std::string lexicographic_paths(const std::vector<std::vector<int>> &grid,
                                int start_val, int end_val, int64_t k) {
    int rows = grid.size(), cols = grid[0].size();
    int sr = -1, sc = -1;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (grid[r][c] == start_val) {
                sr = r;
                sc = c;
            }

    std::vector<std::vector<int64_t>> dp(rows, std::vector<int64_t>(cols, -1));
    PathCounter pc{grid, dp, rows, cols, end_val};

    int64_t counted = 0;
    std::string path;
    path.reserve(rows * cols);
    pc.trace(sr, sc, k, counted, path);
    return path;
}

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


int main(int argc,char* argv[]){
    if (argc!=3){ std::cerr<<"Usage: spust <input_file> <mode: opt|brute>\n"; return 1; }
    std::ifstream f(argv[1]);
    if (!f){ std::cerr<<"Cannot open: "<<argv[1]<<"\n"; return 1; }
    int64_t N,M,K; f>>N>>M>>K;
    std::vector<std::vector<int>> grid(N,std::vector<int>(M));
    int mn=INT_MAX,mx=INT_MIN;
    for (int i=0;i<N;i++) for (int j=0;j<M;j++){
        f>>grid[i][j]; mn=std::min(mn,grid[i][j]); mx=std::max(mx,grid[i][j]);
    }
    std::string mode=argv[2];
    std::string result = (mode=="brute") ? brute_force(grid,mx,mn,K)
                                         : lexicographic_paths(grid,mx,mn,K);
    std::cout<<result<<"\n";
    return 0;
}
