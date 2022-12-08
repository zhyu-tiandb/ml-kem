#pragma once
#include "decryption.hpp"
#include "encryption.hpp"
#include "pke_keygen.hpp"
#include "utils.hpp"
#include "x86_64_cpu_cycles.hpp"
#include <benchmark/benchmark.h>

// Benchmark Kyber PQC suite implementation on CPU, using google-benchmark
namespace bench_kyber {

// Benchmarking IND-CPA-secure Kyber PKE key generation algorithm
template<const size_t k, const size_t eta1>
void
pke_keygen(benchmark::State& state)
{
  constexpr size_t slen = 32;
  constexpr size_t pklen = k * 12 * 32 + 32;
  constexpr size_t sklen = k * 12 * 32;

  uint8_t* seed = static_cast<uint8_t*>(std::malloc(slen));
  uint8_t* pkey = static_cast<uint8_t*>(std::malloc(pklen));
  uint8_t* skey = static_cast<uint8_t*>(std::malloc(sklen));

  std::memset(pkey, 0, pklen);
  std::memset(skey, 0, sklen);
  kyber_utils::random_data<uint8_t>(seed, slen);

#if defined __x86_64__
  uint64_t total_cycles = 0ul;
#endif

  for (auto _ : state) {
#if defined __x86_64__
    const uint64_t start = cpu_cycles();
#endif

    cpapke::keygen<k, eta1>(seed, pkey, skey);

#if defined __x86_64__
    const uint64_t end = cpu_cycles();
    total_cycles += (end - start);
#endif

    benchmark::DoNotOptimize(seed);
    benchmark::DoNotOptimize(pkey);
    benchmark::DoNotOptimize(skey);
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));

#if defined __x86_64__
  total_cycles /= static_cast<uint64_t>(state.iterations());
  state.counters["average_cpu_cycles"] = static_cast<double>(total_cycles);
#endif

  std::free(seed);
  std::free(pkey);
  std::free(skey);
}

// Benchmarking IND-CPA-secure Kyber public key encryption algorithm
template<const size_t k,
         const size_t eta1,
         const size_t eta2,
         const size_t du,
         const size_t dv>
void
encrypt(benchmark::State& state)
{
  constexpr size_t slen = 32;
  constexpr size_t pklen = k * 12 * 32 + 32;
  constexpr size_t sklen = k * 12 * 32;
  constexpr size_t mlen = 32;
  constexpr size_t enclen = k * du * 32 + dv * 32;

  uint8_t* seed = static_cast<uint8_t*>(std::malloc(slen));
  uint8_t* pkey = static_cast<uint8_t*>(std::malloc(pklen));
  uint8_t* skey = static_cast<uint8_t*>(std::malloc(sklen));
  uint8_t* rcoin = static_cast<uint8_t*>(std::malloc(mlen));
  uint8_t* txt = static_cast<uint8_t*>(std::malloc(mlen));
  uint8_t* enc = static_cast<uint8_t*>(std::malloc(enclen));

  std::memset(pkey, 0, pklen);
  std::memset(skey, 0, sklen);
  std::memset(enc, 0, enclen);

  kyber_utils::random_data<uint8_t>(seed, slen);
  kyber_utils::random_data<uint8_t>(txt, mlen);
  kyber_utils::random_data<uint8_t>(rcoin, mlen);

  cpapke::keygen<k, eta1>(seed, pkey, skey);

#if defined __x86_64__
  uint64_t total_cycles = 0ul;
#endif

  for (auto _ : state) {
#if defined __x86_64__
    const uint64_t start = cpu_cycles();
#endif

    cpapke::encrypt<k, eta1, eta2, du, dv>(pkey, txt, rcoin, enc);

#if defined __x86_64__
    const uint64_t end = cpu_cycles();
    total_cycles += (end - start);
#endif

    benchmark::DoNotOptimize(pkey);
    benchmark::DoNotOptimize(txt);
    benchmark::DoNotOptimize(rcoin);
    benchmark::DoNotOptimize(enc);
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));

#if defined __x86_64__
  total_cycles /= static_cast<uint64_t>(state.iterations());
  state.counters["average_cpu_cycles"] = static_cast<double>(total_cycles);
#endif

  std::free(seed);
  std::free(pkey);
  std::free(skey);
  std::free(rcoin);
  std::free(txt);
  std::free(enc);
}

// Benchmarking IND-CPA-secure Kyber public key decryption algorithm
template<const size_t k,
         const size_t eta1,
         const size_t eta2,
         const size_t du,
         const size_t dv>
void
decrypt(benchmark::State& state)
{
  constexpr size_t slen = 32;
  constexpr size_t pklen = k * 12 * 32 + 32;
  constexpr size_t sklen = k * 12 * 32;
  constexpr size_t mlen = 32;
  constexpr size_t enclen = k * du * 32 + dv * 32;

  uint8_t* seed = static_cast<uint8_t*>(std::malloc(slen));
  uint8_t* pkey = static_cast<uint8_t*>(std::malloc(pklen));
  uint8_t* skey = static_cast<uint8_t*>(std::malloc(sklen));
  uint8_t* rcoin = static_cast<uint8_t*>(std::malloc(mlen));
  uint8_t* txt = static_cast<uint8_t*>(std::malloc(mlen));
  uint8_t* enc = static_cast<uint8_t*>(std::malloc(enclen));
  uint8_t* dec = static_cast<uint8_t*>(std::malloc(mlen));

  std::memset(pkey, 0, pklen);
  std::memset(skey, 0, sklen);
  std::memset(enc, 0, enclen);
  std::memset(dec, 0, mlen);

  kyber_utils::random_data<uint8_t>(seed, slen);
  kyber_utils::random_data<uint8_t>(txt, mlen);
  kyber_utils::random_data<uint8_t>(rcoin, mlen);

  cpapke::keygen<k, eta1>(seed, pkey, skey);
  cpapke::encrypt<k, eta1, eta2, du, dv>(pkey, txt, rcoin, enc);

#if defined __x86_64__
  uint64_t total_cycles = 0ul;
#endif

  for (auto _ : state) {
#if defined __x86_64__
    const uint64_t start = cpu_cycles();
#endif

    cpapke::decrypt<k, du, dv>(skey, enc, dec);

#if defined __x86_64__
    const uint64_t end = cpu_cycles();
    total_cycles += (end - start);
#endif

    benchmark::DoNotOptimize(skey);
    benchmark::DoNotOptimize(enc);
    benchmark::DoNotOptimize(dec);
    benchmark::ClobberMemory();
  }

  for (size_t i = 0; i < mlen; i++) {
    assert(!static_cast<bool>(txt[i] ^ dec[i]));
  }

  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));

#if defined __x86_64__
  total_cycles /= static_cast<uint64_t>(state.iterations());
  state.counters["average_cpu_cycles"] = static_cast<double>(total_cycles);
#endif

  std::free(seed);
  std::free(pkey);
  std::free(skey);
  std::free(rcoin);
  std::free(txt);
  std::free(enc);
  std::free(dec);
}

}
