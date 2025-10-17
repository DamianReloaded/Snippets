// toy_transformer.cpp
// Minimal transformer-like language model (single-head, single-block) that learns next-character prediction
// for the toy sequence "12345678901234567890".
// Note: educational implementation, not production-grade.

#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <execution>
#include <ranges>
#include <thread>
#include <algorithm>

# define M_PI           3.14159265358979323846  /* pi */

using std::vector;
using std::size_t;

// Utility RNG
static std::mt19937 rng(1234);

// ---------- small helpers ----------
void save_vector(std::ofstream& out, const vector<double>& v)
{
    size_t n = v.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof(size_t));
    out.write(reinterpret_cast<const char*>(v.data()), n * sizeof(double));
}

void load_vector(std::ifstream& in, vector<double>& v)
{
    size_t n;
    in.read(reinterpret_cast<char*>(&n), sizeof(size_t));
    v.resize(n);
    in.read(reinterpret_cast<char*>(v.data()), n * sizeof(double));
}

double randn(double stddev)
{
    static std::normal_distribution<double> dist(0.0, 1.0);
    return dist(rng) * stddev;
}

vector<double> zeros(size_t n)
{
    return vector<double>(n, 0.0);
}

double sqr(double x)
{
    return x * x;
}

// ---------- Linear layer (forward + backward, parallelized) ----------
struct Linear
{
    int in_dim;
    int out_dim;
    vector<double> W;    // out_dim x in_dim
    vector<double> b;    // out_dim
    vector<double> dW;
    vector<double> db;
    vector<double> X_cache;
    int rows_cached;

    Linear(int in_dim_, int out_dim_, double init_std = 0.02)
        : in_dim(in_dim_), out_dim(out_dim_)
    {
        W = zeros((size_t)out_dim * in_dim);
        b = zeros((size_t)out_dim);
        dW = zeros((size_t)out_dim * in_dim);
        db = zeros((size_t)out_dim);
        for (int i = 0; i < out_dim; ++i)
            for (int j = 0; j < in_dim; ++j)
                W[(size_t)i * in_dim + j] = randn(init_std);
        for (int i = 0; i < out_dim; ++i)
            b[i] = 0.0;
    }

    vector<double> forward(const vector<double>& X, int rows)
    {
        rows_cached = rows;
        X_cache = X;
        vector<double> Y((size_t)rows * out_dim, 0.0);

        std::for_each(std::execution::par, Y.begin(), Y.end(), [&](double &){});

        for (int r = 0; r < rows; ++r)
        {
            for (int o = 0; o < out_dim; ++o)
            {
                double acc = 0.0;
                for (int i = 0; i < in_dim; ++i)
                    acc += X[(size_t)r * in_dim + i] * W[(size_t)o * in_dim + i];
                Y[(size_t)r * out_dim + o] = acc + b[o];
            }
        }
        return Y;
    }

    vector<double> backward(const vector<double>& dY)
    {
        int rows = rows_cached;
        vector<double> dX((size_t)rows * in_dim, 0.0);

        std::for_each(std::execution::par, dX.begin(), dX.end(), [&](double &){});

        for (int r = 0; r < rows; ++r)
        {
            for (int o = 0; o < out_dim; ++o)
            {
                double grad = dY[(size_t)r * out_dim + o];
                db[o] += grad;
                for (int i = 0; i < in_dim; ++i)
                {
                    dW[(size_t)o * in_dim + i] += grad * X_cache[(size_t)r * in_dim + i];
                    dX[(size_t)r * in_dim + i] += grad * W[(size_t)o * in_dim + i];
                }
            }
        }
        return dX;
    }

    void zero_grad()
    {
        std::fill(dW.begin(), dW.end(), 0.0);
        std::fill(db.begin(), db.end(), 0.0);
    }

    void step(double lr)
    {
        for (size_t i = 0; i < W.size(); ++i) W[i] -= lr * dW[i];
        for (size_t i = 0; i < b.size(); ++i) b[i] -= lr * db[i];
    }
};

void save_linear(std::ofstream& out, const Linear& L)
{
    out.write(reinterpret_cast<const char*>(&L.in_dim), sizeof(int));
    out.write(reinterpret_cast<const char*>(&L.out_dim), sizeof(int));
    save_vector(out, L.W);
    save_vector(out, L.b);
}
void load_linear(std::ifstream& in, Linear& L)
{
    int in_dim, out_dim;
    in.read(reinterpret_cast<char*>(&in_dim), sizeof(int));
    in.read(reinterpret_cast<char*>(&out_dim), sizeof(int));
    load_vector(in, L.W);
    load_vector(in, L.b);
    L.in_dim = in_dim;
    L.out_dim = out_dim;
}    

// ---------- Embedding (forward + backward) ----------
struct Embedding
{
    int vocab;
    int dim;
    vector<double> E;   // vocab x dim (row-major: E[v*dim + d])
    vector<double> dE;
    // cache
    vector<int> tokens_cache;
    int cached_rows;

    Embedding(int vocab_, int dim_, double init_std = 0.02)
    {
        vocab = vocab_;
        dim = dim_;
        E = zeros((size_t)vocab * dim);
        dE = zeros((size_t)vocab * dim);
        for (int v = 0; v < vocab; ++v)
        {
            for (int d = 0; d < dim; ++d)
            {
                E[v * dim + d] = randn(init_std);
            }
        }
    }

    // tokens: length rows -> returns X: rows x dim
    vector<double> forward(const vector<int>& tokens)
    {
        cached_rows = (int)tokens.size();
        tokens_cache = tokens;
        vector<double> X((size_t)cached_rows * dim);
        for (int r = 0; r < cached_rows; ++r)
        {
            int t = tokens[r];
            for (int d = 0; d < dim; ++d)
            {
                X[(size_t)r * dim + d] = E[(size_t)t * dim + d];
            }
        }
        return X;
    }

    // dX: rows x dim -> accumulate into dE
    void backward(const vector<double>& dX)
    {
        for (int r = 0; r < cached_rows; ++r)
        {
            int t = tokens_cache[r];
            for (int d = 0; d < dim; ++d)
            {
                dE[(size_t)t * dim + d] += dX[(size_t)r * dim + d];
            }
        }
    }

    void zero_grad()
    {
        std::fill(dE.begin(), dE.end(), 0.0);
    }

    void step(double lr)
    {
        for (size_t i = 0; i < E.size(); ++i)
        {
            E[i] -= lr * dE[i];
        }
    }
};

void save_embedding(std::ofstream& out, const Embedding& E)
{
    out.write(reinterpret_cast<const char*>(&E.vocab), sizeof(int));
    out.write(reinterpret_cast<const char*>(&E.dim), sizeof(int));
    save_vector(out, E.E);
}
void load_embedding(std::ifstream& in, Embedding& E)
{
    int vocab, dim;
    in.read(reinterpret_cast<char*>(&vocab), sizeof(int));
    in.read(reinterpret_cast<char*>(&dim), sizeof(int));
    load_vector(in, E.E);
    E.vocab = vocab;
    E.dim = dim;
    E.dE = zeros((size_t)vocab * dim);
}


// ---------- LayerNorm (per-token, across features) ----------
struct LayerNorm
{
    int dim;
    vector<double> gamma;
    vector<double> beta;
    vector<double> dgamma;
    vector<double> dbeta;
    vector<double> X_cache;
    vector<double> mean_cache;
    vector<double> var_cache;
    int rows_cached;
    const double eps = 1e-5;

    LayerNorm(int dim_)
        : dim(dim_)
    {
        gamma = vector<double>(dim, 1.0);
        beta = vector<double>(dim, 0.0);
        dgamma = zeros(dim);
        dbeta = zeros(dim);
    }

    vector<double> forward(const vector<double>& X, int rows)
    {
        rows_cached = rows;
        X_cache = X;
        mean_cache.resize(rows);
        var_cache.resize(rows);
        vector<double> Y((size_t)rows * dim);

        std::for_each(std::execution::par, Y.begin(), Y.end(), [&](double &){});

        for (int r = 0; r < rows; ++r)
        {
            double mean = 0.0;
            for (int d = 0; d < dim; ++d) mean += X[(size_t)r * dim + d];
            mean /= dim;
            mean_cache[r] = mean;

            double var = 0.0;
            for (int d = 0; d < dim; ++d)
            {
                double diff = X[(size_t)r * dim + d] - mean;
                var += diff * diff;
            }
            var /= dim;
            var_cache[r] = var;

            for (int d = 0; d < dim; ++d)
            {
                Y[(size_t)r * dim + d] = gamma[d] * (X[(size_t)r * dim + d] - mean) / sqrt(var + eps) + beta[d];
            }
        }
        return Y;
    }

    vector<double> backward(const vector<double>& dY)
    {
        int rows = rows_cached;
        vector<double> dX((size_t)rows * dim, 0.0);

        std::for_each(std::execution::par, dX.begin(), dX.end(), [&](double &){});

        for (int r = 0; r < rows; ++r)
        {
            double var = var_cache[r] + eps;
            double sqrt_var = sqrt(var);
            for (int d = 0; d < dim; ++d)
            {
                double x_hat = (X_cache[(size_t)r * dim + d] - mean_cache[r]) / sqrt_var;
                dgamma[d] += dY[(size_t)r * dim + d] * x_hat;
                dbeta[d] += dY[(size_t)r * dim + d];
            }

            for (int d = 0; d < dim; ++d)
            {
                double dxhat = dY[(size_t)r * dim + d] * gamma[d];
                double dvar = -0.5 * dxhat * (X_cache[(size_t)r * dim + d] - mean_cache[r]) / (var * sqrt_var);
                double dmean = -dxhat / sqrt_var;
                dX[(size_t)r * dim + d] = dxhat / sqrt_var + dvar * 2.0 * (X_cache[(size_t)r * dim + d] - mean_cache[r]) / dim + dmean / dim;
            }
        }
        return dX;
    }

    void zero_grad()
    {
        std::fill(dgamma.begin(), dgamma.end(), 0.0);
        std::fill(dbeta.begin(), dbeta.end(), 0.0);
    }

    void step(double lr)
    {
        for (int d = 0; d < dim; ++d)
        {
            gamma[d] -= lr * dgamma[d];
            beta[d] -= lr * dbeta[d];
        }
    }
};

void save_layernorm(std::ofstream& out, const LayerNorm& LN)
{
    out.write(reinterpret_cast<const char*>(&LN.dim), sizeof(int));
    save_vector(out, LN.gamma);
    save_vector(out, LN.beta);
}
void load_layernorm(std::ifstream& in, LayerNorm& LN)
{
    int dim;
    in.read(reinterpret_cast<char*>(&dim), sizeof(int));
    load_vector(in, LN.gamma);
    load_vector(in, LN.beta);
    LN.dim = dim;
    LN.dgamma = zeros(dim);
    LN.dbeta = zeros(dim);
}    

// ---------- Activation: GELU and derivative ----------
double gelu(double x)
{
    // approximate GELU using tanh
    const double a = sqrt(2.0 / M_PI);
    double x3 = x * x * x;
    double inner = a * (x + 0.044715 * x3);
    return 0.5 * x * (1.0 + tanh(inner));
}

double gelu_grad(double x)
{
    // derivative via numeric-ish formula (safe)
    const double a = sqrt(2.0 / M_PI);
    double x2 = x * x;
    double inner = a * (x + 0.044715 * x * x2);
    double t = tanh(inner);
    double dt_dx = a * (1.0 + 0.044715 * 3.0 * x * x);
    double grad = 0.5 * (1.0 + t + x * (1.0 - t * t) * dt_dx);
    return grad;
}

// ---------- Single-head causal self-attention (forward + backward) ----------

struct SelfAttention
{
    int dim;
    int d_k;
    Linear Wq, Wk, Wv, Wo;
    int rows_cached;
    vector<double> Q, K, V, scores, attn;
    double scale;

    SelfAttention(int dim_) : dim(dim_), d_k(dim_), Wq(dim, dim), Wk(dim, dim), Wv(dim, dim), Wo(dim, dim)
    {
        scale = 1.0 / sqrt((double)d_k);
        rows_cached = 0;
    }

    vector<double> forward(const vector<double>& X, int rows)
    {
        rows_cached = rows;
        Q = Wq.forward(X, rows);
        K = Wk.forward(X, rows);
        V = Wv.forward(X, rows);

        scores.assign((size_t)rows * rows, 0.0);
        attn.assign((size_t)rows * rows, 0.0);

        // compute scores with causal mask
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j <= i; ++j)
            {
                double acc = 0.0;
                for (int k = 0; k < d_k; ++k) acc += Q[(size_t)i * d_k + k] * K[(size_t)j * d_k + k];
                scores[(size_t)i * rows + j] = acc * scale;
            }
            for (int j = i + 1; j < rows; ++j) scores[(size_t)i * rows + j] = -1e9;
        }

        // softmax per row
        for (int i = 0; i < rows; ++i)
        {
            double mx = -1e18, sum = 0.0;
            for (int j = 0; j < rows; ++j) if (scores[(size_t)i * rows + j] > mx) mx = scores[(size_t)i * rows + j];
            for (int j = 0; j < rows; ++j)
            {
                attn[(size_t)i * rows + j] = exp(scores[(size_t)i * rows + j] - mx);
                sum += attn[(size_t)i * rows + j];
            }
            for (int j = 0; j < rows; ++j) attn[(size_t)i * rows + j] /= (sum == 0.0 ? 1e-12 : sum);
        }

        vector<double> out((size_t)rows * dim, 0.0);
        for (int i = 0; i < rows; ++i)
            for (int d = 0; d < dim; ++d)
            {
                double acc = 0.0;
                for (int j = 0; j < rows; ++j) acc += attn[(size_t)i * rows + j] * V[(size_t)j * dim + d];
                out[(size_t)i * dim + d] = acc;
            }

        return Wo.forward(out, rows);
    }

    vector<double> backward(const vector<double>& dOutProj)
    {
        // same as previous parallel-compatible backward
        int rows = rows_cached;
        vector<double> dOut = Wo.backward(dOutProj);
        vector<double> dAttn((size_t)rows * rows, 0.0), dV((size_t)rows * dim, 0.0);
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < rows; ++j)
            {
                double dot = 0.0;
                for (int d = 0; d < dim; ++d) dot += dOut[(size_t)i * dim + d] * V[(size_t)j * dim + d];
                dAttn[(size_t)i * rows + j] = dot;
            }
            for (int j = 0; j < rows; ++j)
                for (int d = 0; d < dim; ++d) dV[(size_t)j * dim + d] += attn[(size_t)i * rows + j] * dOut[(size_t)i * dim + d];
        }

        vector<double> dScores((size_t)rows * rows, 0.0);
        for (int i = 0; i < rows; ++i)
        {
            double dot = 0.0;
            for (int j = 0; j < rows; ++j) dot += attn[(size_t)i * rows + j] * dAttn[(size_t)i * rows + j];
            for (int j = 0; j < rows; ++j) dScores[(size_t)i * rows + j] = (attn[(size_t)i * rows + j] * (dAttn[(size_t)i * rows + j] - dot)) * scale;
        }

        vector<double> dQ((size_t)rows * d_k, 0.0), dK((size_t)rows * d_k, 0.0);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < rows; ++j)
            {
                double s = dScores[(size_t)i * rows + j];
                if (s == 0.0) continue;
                for (int k = 0; k < d_k; ++k)
                {
                    dQ[(size_t)i * d_k + k] += s * K[(size_t)j * d_k + k];
                    dK[(size_t)j * d_k + k] += s * Q[(size_t)i * d_k + k];
                }
            }

        vector<double> dX = Wv.backward(dV);
        vector<double> dXQ = Wq.backward(dQ);
        vector<double> dXK = Wk.backward(dK);

        for (size_t i = 0; i < dX.size(); ++i) dX[i] += dXQ[i] + dXK[i];
        return dX;
    }

    void zero_grad() { Wq.zero_grad(); Wk.zero_grad(); Wv.zero_grad(); Wo.zero_grad(); }
    void step(double lr) { Wq.step(lr); Wk.step(lr); Wv.step(lr); Wo.step(lr); }
};

// ---------- Feed-forward MLP (two-layer) ----------
struct MLP
{
    Linear fc1; // dim -> d_ff
    Linear fc2; // d_ff -> dim
    int rows_cached;
    vector<double> hidden_cache; // rows x d_ff

    MLP(int dim, int d_ff)
        : fc1(dim, d_ff, 0.02), fc2(d_ff, dim, 0.02)
    {
    }

    vector<double> forward(const vector<double>& X, int rows)
    {
        rows_cached = rows;
        vector<double> h = fc1.forward(X, rows);
        hidden_cache = h;

        std::for_each(std::execution::par, h.begin(), h.end(), [&](double& val)
        {
            val = gelu(val);
        });

        vector<double> out = fc2.forward(h, rows);
        return out;
    }

    vector<double> backward(const vector<double>& dOut)
    {
        vector<double> dh = fc2.backward(dOut);
        std::for_each(std::execution::par, dh.begin(), dh.end(), [&](double& val)
        {
            size_t idx = &val - &dh[0];
            val *= gelu_grad(hidden_cache[idx]);
        });
        vector<double> dX = fc1.backward(dh);
        return dX;
    }

    void zero_grad()
    {
        fc1.zero_grad();
        fc2.zero_grad();
    }

    void step(double lr)
    {
        fc1.step(lr);
        fc2.step(lr);
    }
};

// ---------- Transformer Block (LN -> SelfAttn -> Residual -> LN -> MLP -> Residual) ----------
struct TransformerBlock
{
    int dim;
    SelfAttention sa;
    LayerNorm ln1;
    LayerNorm ln2;
    MLP mlp;

    TransformerBlock(int dim_, int d_ff)
        : dim(dim_), sa(dim_), ln1(dim_), ln2(dim_), mlp(dim_, d_ff)
    {
    }

    vector<double> forward(const vector<double>& X, int rows)
    {
        vector<double> x_ln1 = ln1.forward(X, rows);
        vector<double> sa_out = sa.forward(x_ln1, rows);

        vector<double> res1((size_t)rows * dim);
        std::transform(std::execution::par, X.begin(), X.end(), sa_out.begin(), res1.begin(),
                       [](double a, double b){ return a + b; });

        vector<double> x_ln2 = ln2.forward(res1, rows);
        vector<double> mlp_out = mlp.forward(x_ln2, rows);

        vector<double> out((size_t)rows * dim);
        std::transform(std::execution::par, res1.begin(), res1.end(), mlp_out.begin(), out.begin(),
                       [](double a, double b){ return a + b; });

        return out;
    }

    vector<double> backward(const vector<double>& dOut)
    {
        int rows = sa.rows_cached;
        vector<double> dres1((size_t)rows * dim);
        vector<double> dmlp_out((size_t)rows * dim);

        std::transform(std::execution::par, dOut.begin(), dOut.end(), dOut.begin(), dres1.begin(),
                       [](double a, double b){ return a; }); // dres1 = dOut
        std::transform(std::execution::par, dOut.begin(), dOut.end(), dOut.begin(), dmlp_out.begin(),
                       [](double a, double b){ return a; }); // dmlp_out = dOut

        vector<double> dx_ln2 = mlp.backward(dmlp_out);
        vector<double> dres1_from_ln2 = ln2.backward(dx_ln2);

        std::for_each(std::execution::par, dres1.begin(), dres1.end(), [&](double& val)
        {
            size_t idx = &val - &dres1[0];
            val += dres1_from_ln2[idx];
        });

        vector<double> dX_from_res1 = dres1;
        vector<double> dsa_out = dres1;

        vector<double> dx_ln1 = sa.backward(dsa_out);
        vector<double> dX_from_ln1 = ln1.backward(dx_ln1);

        vector<double> dX((size_t)rows * dim);
        std::transform(std::execution::par, dX_from_res1.begin(), dX_from_res1.end(),
                       dX_from_ln1.begin(), dX.begin(),
                       [](double a, double b){ return a + b; });

        return dX;
    }

    void zero_grad()
    {
        sa.zero_grad();
        ln1.zero_grad();
        ln2.zero_grad();
        mlp.zero_grad();
    }

    void step(double lr)
    {
        sa.step(lr);
        ln1.step(lr);
        ln2.step(lr);
        mlp.step(lr);
    }
};

// ---------- Simple model: Embedding + Positional (learned) + N blocks + final linear head ----------
struct SimpleTransformer
{
    int vocab;
    int dim;
    int seq_len;
    Embedding token_emb;
    Embedding pos_emb;
    TransformerBlock block;
    Linear head; // dim -> vocab

    SimpleTransformer(int vocab_, int dim_, int seq_len_, int d_ff)
        : vocab(vocab_), dim(dim_), seq_len(seq_len_),
          token_emb(vocab_, dim_, 0.1), pos_emb(seq_len_, dim_, 0.1),
          block(dim_, d_ff), head(dim_, vocab_, 0.02)
    {
    }

    vector<double> forward(const vector<int>& tokens)
    {
        vector<double> Xe = token_emb.forward(tokens); // seq_len x dim

        vector<int> pos_idx(seq_len);
        for (int i = 0; i < seq_len; ++i) pos_idx[i] = i;
        vector<double> Xp = pos_emb.forward(pos_idx);

        vector<double> X((size_t)seq_len * dim);
        std::transform(std::execution::par, Xe.begin(), Xe.end(), Xp.begin(), X.begin(),
                       [](double a, double b){ return a + b; });

        vector<double> hidden = block.forward(X, seq_len);
        vector<double> logits = head.forward(hidden, seq_len);
        return logits;
    }

    void backward(const vector<double>& dLogits)
    {
        vector<double> dh = head.backward(dLogits);
        vector<double> dX = block.backward(dh);

        // propagate gradients to embeddings
        token_emb.backward(dX);
        pos_emb.backward(dX);
    }

    void zero_grad()
    {
        token_emb.zero_grad();
        pos_emb.zero_grad();
        block.zero_grad();
        head.zero_grad();
    }

    void step(double lr)
    {
        token_emb.step(lr);
        pos_emb.step(lr);
        block.step(lr);
        head.step(lr);
    }
};



void save_model(const SimpleTransformer& M, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    if (!out) throw std::runtime_error("Could not open file for saving");

    out.write(reinterpret_cast<const char*>(&M.vocab), sizeof(int));
    out.write(reinterpret_cast<const char*>(&M.dim), sizeof(int));
    out.write(reinterpret_cast<const char*>(&M.seq_len), sizeof(int));

    save_embedding(out, M.token_emb);
    save_embedding(out, M.pos_emb);

    // Transformer block
    save_layernorm(out, M.block.ln1);
    save_layernorm(out, M.block.ln2);

    save_linear(out, M.block.sa.Wq);
    save_linear(out, M.block.sa.Wk);
    save_linear(out, M.block.sa.Wv);
    save_linear(out, M.block.sa.Wo);

    save_linear(out, M.block.mlp.fc1);
    save_linear(out, M.block.mlp.fc2);

    save_linear(out, M.head);

    out.close();
}

void load_model(SimpleTransformer& M, const std::string& filename)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in) throw std::runtime_error("Could not open file for loading");

    int vocab, dim, seq_len;
    in.read(reinterpret_cast<char*>(&vocab), sizeof(int));
    in.read(reinterpret_cast<char*>(&dim), sizeof(int));
    in.read(reinterpret_cast<char*>(&seq_len), sizeof(int));

    if (vocab != M.vocab || dim != M.dim || seq_len != M.seq_len)
        throw std::runtime_error("Model dimensions mismatch when loading");

    load_embedding(in, M.token_emb);
    load_embedding(in, M.pos_emb);

    load_layernorm(in, M.block.ln1);
    load_layernorm(in, M.block.ln2);

    load_linear(in, M.block.sa.Wq);
    load_linear(in, M.block.sa.Wk);
    load_linear(in, M.block.sa.Wv);
    load_linear(in, M.block.sa.Wo);

    load_linear(in, M.block.mlp.fc1);
    load_linear(in, M.block.mlp.fc2);

    load_linear(in, M.head);

    in.close();
}

// ---------- Cross-entropy loss + softmax derivative (per row) ----------
double cross_entropy_loss_and_grad(const vector<double>& logits, int rows, int vocab, const vector<int>& targets, vector<double>& dLogits_out)
{
    dLogits_out.assign((size_t)rows * vocab, 0.0);
    double loss = 0.0;
    for (int r = 0; r < rows; ++r)
    {
        double mx = -1e18;
        for (int v = 0; v < vocab; ++v)
        {
            double val = logits[(size_t)r * vocab + v];
            if (val > mx) mx = val;
        }
        double sum = 0.0;
        for (int v = 0; v < vocab; ++v)
        {
            double ex = exp(logits[(size_t)r * vocab + v] - mx);
            dLogits_out[(size_t)r * vocab + v] = ex;
            sum += ex;
        }
        for (int v = 0; v < vocab; ++v)
        {
            dLogits_out[(size_t)r * vocab + v] /= sum;
        }
        int t = targets[r];
        double p_t = dLogits_out[(size_t)r * vocab + t];
        loss += -log(std::max(p_t, 1e-12));
        dLogits_out[(size_t)r * vocab + t] -= 1.0;
    }
    loss /= rows;
    for (size_t i = 0; i < dLogits_out.size(); ++i)
    {
        dLogits_out[i] /= rows;
    }
    return loss;
}

// ------------------- SAMPLING HELPER -------------------
int sample_next(const vector<double>& logits_row, int vocab)
{
    // softmax + multinomial sample
    double mx = -1e18;
    for (int v = 0; v < vocab; ++v)
        if (logits_row[v] > mx) mx = logits_row[v];
    vector<double> probs(vocab);
    double sum = 0.0;
    for (int v = 0; v < vocab; ++v)
    {
        probs[v] = exp(logits_row[v] - mx);
        sum += probs[v];
    }
    for (int v = 0; v < vocab; ++v) probs[v] /= sum;

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r = dist(rng);
    double cum = 0.0;
    for (int v = 0; v < vocab; ++v)
    {
        cum += probs[v];
        if (r <= cum) return v;
    }
    return vocab - 1;
}

// ---------- MAIN: build data, model, train ----------
int main()
{
    // Example poem text (replace with a real poem, can be hundreds of chars)
    std::string text = R"(So that from point to point now have you heard
The fundamental reasons of this war,
Whose great decision hath much blood let forth
And more thirsts after.
Holy seems the quarrel
Upon your grace's part; black and fearful
On the opposer.
Therefore we marvel much our cousin France
Would in so just a business shut his bosom
Against our borrowing prayers.
Second Lord
Good my lord,
The reasons of our state I cannot yield,
But like a common and an outward man,
That the great figure of a council frames
By self-unable motion: therefore dare not
Say what I think of it, since I have found
Myself in my incertain grounds to fail
As often as I guess'd.
Be it his pleasure.
But I am sure the younger of our nature,
That surfeit on their ease, will day by day
Come here for physic.
Welcome shall they be;
And all the honours that can fly from us
Shall on them settle. You know your places well;
When better fall, for your avails they fell:
To-morrow to the field.)";
    

    // Convert to tokens 0..255
    vector<int> tokens(text.size());
    for (size_t i = 0; i < text.size(); ++i)
        tokens[i] = (unsigned char)text[i];

    // Hyperparams
    int vocab = 256;
    int d_model = 64;
    int d_ff = 128;
    int seq_len = 64;      // context window length
    int epochs = 20000;     // training iterations
    double lr = 5e-3;

    SimpleTransformer model(vocab, d_model, seq_len, d_ff);

    bool must_train = false;
    std::string fname = "poem_model.bin";

    try
    {
        load_model(model, fname);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error Loading Model: " << e.what() << '\n';
    }
    

    if (must_train)
    {
        // TRAINING LOOP
        std::uniform_int_distribution<int> start_dist(0, (int)tokens.size() - seq_len - 1);
        for (int e = 0; e < epochs; ++e)
        {
            // pick random subsequence
            int start = start_dist(rng);
            vector<int> input_tokens(seq_len), target_tokens(seq_len);
            for (int i = 0; i < seq_len; ++i)
            {
                input_tokens[i] = tokens[start + i];
                target_tokens[i] = tokens[start + i + 1];
            }

            // forward, loss, backward, update
            vector<double> logits = model.forward(input_tokens);
            vector<double> dLogits;
            double loss = cross_entropy_loss_and_grad(logits, seq_len, vocab, target_tokens, dLogits);
            model.zero_grad();
            model.backward(dLogits);
            model.step(lr);

            if (e % 100 == 0)
            {
                std::cout << "Epoch " << e << " loss=" << loss << "\n";
            }
        }

        try
        {
            save_model(model, fname);
            std::cout << "Model saved to " << fname << "\n";
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error Saving Model: " << e.what() << '\n';
        }
    
    }

    // GENERATION
    std::cout << "\n--- Generated Poem ---\n";
    int gen_len = 2000; // characters to generate
    vector<int> context(seq_len, ' '); // start with spaces
    // pick random starting char
    std::uniform_int_distribution<int> char_dist(0, 255);
    context[seq_len - 1] = char_dist(rng);

    for (int i = 0; i < gen_len; ++i)
    {
        vector<double> logits = model.forward(context);
        // take last position prediction
        vector<double> row(logits.begin() + (seq_len - 1) * vocab,
                           logits.begin() + seq_len * vocab);
        int next_char = sample_next(row, vocab);

        std::cout << (char)next_char;

        // shift context left, append next_char
        for (int j = 0; j < seq_len - 1; ++j)
            context[j] = context[j + 1];
        context[seq_len - 1] = next_char;
    }

    std::cout << "\n----------------------\n";

    std::cin.get();
    return 0;
}

/*
#include <csignal>
#include <iostream>
#include <memory>
#include "Application/Application.hpp"

int RunApplication()
{
    Game::Application app;
    try
    {
        app.Run();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ERROR] Exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[ERROR] Unknown exception occurred" << std::endl;
    }

    app.TerminateManagers();

    return EXIT_FAILURE;
}

void SignalHandler(int signal)
{
    if (signal == SIGABRT)
    {
        std::cerr << "[FATAL] Caught SIGABRT (assertion failure or abort)." << std::endl;
#ifdef DEBUG
        std::cin.get();
#endif
        std::exit(EXIT_FAILURE); // or std::abort(); or std::raise(SIGTRAP);
    }
}

int main(int argc, char* argv[])
{
    std::signal(SIGABRT, SignalHandler);

    int retval = RunApplication();

    std::cout << "Press Enter to exit..." << std::endl;
#ifdef DEBUG
    std::cin.get();
#endif
    return retval;
}
*/