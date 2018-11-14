#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    std::cerr << "specify Lx and Ly\n";
    return -1;
  }
  double lx = atof(argv[1]);
  double ly = atof(argv[2]);
  std::vector<double> kx;
  std::vector<double> ky;
  int n=10;
  for(int i=0; i!=n; i++)
  {
    kx.push_back((double) i * M_PI / lx);
    ky.push_back((double) i * M_PI / ly);
    
  }
  std::vector<double> w;
  for(int i=0;i!=n;i++)
  {
    for(int j=0;j!=n;j++)
      w.push_back(sqrt(kx[i] * kx[i] + ky[j] * ky[j]));
  }

  std::sort(w.begin(), w.end(), [](const double& a, const double& b){return a > b;});
  std::cout << w.size() << '\n';
  for(auto _w : w)
    std::cout << -_w << '\n';
  return 0;
}
