struct BSParams//Data tansfer object
{
    double S;

    double K;

    double r;

    double sigma;

    double T;
};

struct Quote 
{
    double price, delta, gamma, vega;
};


class BlackScholes {
private:
    BSParams p_;
    double norm_cdf(double x) const;
    double norm_pdf(double x) const;
    double d1() const;
    double d2() const;
public:
    explicit BlackScholes(const BSParams& p);
    double call_price() const;
    double put_price()const;
    double delta_call() const;
    double delta_put() const;
    double gamma() const;
    double vega() const; 
};

class Option {
    public: virtual Quote quote() const = 0;
    virtual ~Option() = default;
};

class CallOption : public Option {
    private: const BlackScholes& model_;
    public:
        explicit CallOption(const BlackScholes& model);
        Quote quote() const override;
};

class PutOption : public Option {
    private: const BlackScholes& model_;
    public:
        explicit PutOption(const BlackScholes& model);
        Quote quote() const override;
};