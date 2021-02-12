#ifndef H_HIG_PRODUCER
#define H_HIG_PRODUCER

#include <vector>

#include "TLorentzVector.h"

#include "nano_tree.hpp"
#include "pico_tree.hpp"

struct HiggsConstructionVariables {
  std::vector<TLorentzVector> jet_lv;
  std::vector<float> jet_deepcsv;
};

class HigVarProducer{
public:

  explicit HigVarProducer(int year);
  ~HigVarProducer();

  void WriteHigVars(pico_tree& pico, bool doDeepFlav, bool isFastsim,
                    std::vector<HiggsConstructionVariables> sys_higvars);

private:
  int year;
  
};

#endif
