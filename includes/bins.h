#ifndef __PHOD_BINS__
#define __PHOD_BINS__

#include <vector>
#include <iostream>

#include "xjjcuti.h"

namespace xjjana
{
  template<typename T> class bins
  {
  public:
    bins() { ; }
    bins(const std::vector<T>& vbins) : v_(vbins) { std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl; n_ = v_.size() - 1; print(); }
    int ibin(T x);
    int n() { return n_; }
    const T& operator[](std::size_t i) const { return v_[i]; }
    std::vector<T> v() { return v_; }
    std::string tag(std::size_t i, std::string var, std::string unit = "");
    T width(std::size_t i) { return (v_[i+1]-v_[i]); }
    T area(std::size_t i) { return (v_[i+1]*v_[i+1]-v_[i]*v_[i+1]); }
    void print();
  private:
    std::vector<T> v_;
    int n_;
  };
}

template<typename T>
int xjjana::bins<T>::ibin(T x)
{
  int ielement = -1;
  for(std::size_t i=0;i<n_ && ielement<0;i++)
    {
      if(x>=v_[i] && x<v_[i+1]) ielement = i;
    }
  return ielement;
}

template<typename T>
void xjjana::bins<T>::print()
{
  T x;
  xjjc::prt_divider("\e[34m");
  std::cout << "\e[34m  bins<" << xjjc::gettype(x) << ">({";
  for(auto& i : v_) { std::cout << i << ", "; }
  std::cout << "})" << std::endl;
  xjjc::prt_divider("\e[34m");
}

template<typename T>
std::string xjjana::bins<T>::tag(std::size_t i, std::string var, std::string unit)
{
  std::string t("");
  if(i >= n_) return t;
  t = xjjc::number_remove_zero(v_[i]) + " < " + var + " < " + xjjc::number_remove_zero(v_[i+1]) + unit;
  return t;
}

#endif
