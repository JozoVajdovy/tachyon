// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_BASE_PROVER_H_
#define TACHYON_ZK_BASE_PROVER_H_

#include <memory>
#include <utility>

#include "tachyon/zk/base/blinded_polynomial.h"
#include "tachyon/zk/base/blinder.h"
#include "tachyon/zk/transcript/transcript.h"

namespace tachyon::zk {

template <typename _PCSTy>
class Prover {
 public:
  using PCSTy = _PCSTy;
  using F = typename PCSTy::Field;
  using Domain = typename PCSTy::Domain;
  using Evals = typename PCSTy::Evals;
  using Poly = typename PCSTy::Poly;
  using Commitment = typename PCSTy::Commitment;

  Prover(PCSTy pcs, std::unique_ptr<Domain> domain, Blinder<PCSTy> blinder,
         std::unique_ptr<TranscriptWriter<Commitment>> writer)
      : pcs_(std::move(pcs)),
        domain_(std::move(domain)),
        blinder_(std::move(blinder)),
        writer_(std::move(writer)) {}

  const PCSTy& pcs() const { return pcs_; }
  const Domain* domain() const { return domain_.get(); }
  Blinder<PCSTy>& blinder() { return blinder_; }
  TranscriptWriter<Commitment>* writer() { return writer_.get(); }

  bool CommitLagrange(const Evals& evals) {
    if (evals.NumElements() != domain_->size()) return false;

    Commitment commitment;
    if (!pcs_.CommitLagrange(evals, &commitment)) return false;
    writer_->WriteToProof(commitment);
    return true;
  }

  bool CommitEvalsWithBlind(const Evals& evals, BlindedPolynomial<Poly>* out) {
    if (evals.NumElements() != domain_->size()) return false;

    Commitment commitment;
    if (!pcs_.CommitLagrange(evals, &commitment)) return false;
    writer_->WriteToProof(commitment);

    *out = {domain_->IFFT(evals), blinder_.Generate()};
    return true;
  }

  void Evaluate(const Poly& poly, const F& x) {
    F result = poly.Evaluate(x);
    writer_->WriteToProof(result);
  }

 protected:
  PCSTy pcs_;
  std::unique_ptr<Domain> domain_;
  Blinder<PCSTy> blinder_;
  std::unique_ptr<TranscriptWriter<Commitment>> writer_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_BASE_PROVER_H_
