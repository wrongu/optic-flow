#pragma once

class DoubleFunc
{
	public:
		DoubleFunc(){
			_val = 1.0;
		}
		
		DoubleFunc(double v) : _val(v) {}
		
		DoubleFunc(DoubleFunc & other){
			_val = other._val;
		}
		
		virtual ~DoubleFunc(){}
		
		virtual double operator() (double x){
			return _val;
		}
    private:
		double _val;
};

// gaussians and derivatives
double gauss_exp(double x, double sigma2);
double gauss1_exp(double x, double sigma2);
double gauss2_exp(double x, double sigma2);

// Functor class for performing gaussian derivative for a given sigma
class GaussDerivFunc : public DoubleFunc
{
    public:
        GaussDerivFunc (int nd, double sigma) : _nd(nd), _sigma( sigma ) {}
		~GaussDerivFunc(){}
        double operator() (double x) {
			switch(_nd){
				case 1:
					// 1st deriv
					return gauss1_exp(x, _sigma);
				case 2:
					// 2nd deriv
					return gauss2_exp(x, _sigma);
					break;
				case 0:
				default:
					// 0th is default
					return gauss_exp(x, _sigma);
					break;
			}
			return 0.0;
		}
	private:
		int _nd;
		double _sigma;
};
