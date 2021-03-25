/*
 * Program listing for my blog post "Performance comparison of three different
 * implementations of dynamic_cast" [1]
 *
 * [1]: https://blog.michael.franzl.name/2021/03/21/performance-comparison-of-three-different-implementations-of-dynamic_cast/
 *
 * MIT License
 *
 * Copyright (c) 2021 Michael Karl Franzl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <cstdlib>
#include <typeinfo>
#include <string>
#include <functional>
#include <random>
#include <algorithm>

#include "priori/priori.h"
#include "KCL/KCL_RTTI.h"

enum class Hierarchy { deep, shallow, balanced };
enum class SortOrder { aligned, shuffled };

auto max_num_ops = 0;

const uint64_t n = 2'000'000; // number of iterations
const auto num_usecs_per_sec = 1'000'000;

struct A : priori::Base {
    KCL_RTTI_IMPL();
    uint64_t get() { return x; };
    uint64_t x { 1 };
};
KCL_RTTI_REGISTER(A);

namespace deep {
    struct B : A { KCL_RTTI_IMPL(); B() { priori(this); } };
    struct C : B { KCL_RTTI_IMPL(); C() { priori(this); } };
    struct D : C { KCL_RTTI_IMPL(); D() { priori(this); } };
    struct E : D { KCL_RTTI_IMPL(); E() { priori(this); } };
    struct F : E { KCL_RTTI_IMPL(); F() { priori(this); } };
    struct G : F { KCL_RTTI_IMPL(); G() { priori(this); } };
    struct H : G { KCL_RTTI_IMPL(); H() { priori(this); } };
}
KCL_RTTI_REGISTER(deep::B, A);
KCL_RTTI_REGISTER(deep::C, deep::B);
KCL_RTTI_REGISTER(deep::D, deep::C);
KCL_RTTI_REGISTER(deep::E, deep::D);
KCL_RTTI_REGISTER(deep::F, deep::E);
KCL_RTTI_REGISTER(deep::G, deep::F);
KCL_RTTI_REGISTER(deep::H, deep::G);

namespace shallow {
    struct B : A { KCL_RTTI_IMPL(); B() { priori(this); } };
    struct C : A { KCL_RTTI_IMPL(); C() { priori(this); } };
    struct D : A { KCL_RTTI_IMPL(); D() { priori(this); } };
    struct E : A { KCL_RTTI_IMPL(); E() { priori(this); } };
    struct F : A { KCL_RTTI_IMPL(); F() { priori(this); } };
    struct G : A { KCL_RTTI_IMPL(); G() { priori(this); } };
    struct H : A { KCL_RTTI_IMPL(); H() { priori(this); } };
}
KCL_RTTI_REGISTER(shallow::B, A);
KCL_RTTI_REGISTER(shallow::C, shallow::B);
KCL_RTTI_REGISTER(shallow::D, shallow::C);
KCL_RTTI_REGISTER(shallow::E, shallow::D);
KCL_RTTI_REGISTER(shallow::F, shallow::E);
KCL_RTTI_REGISTER(shallow::G, shallow::F);
KCL_RTTI_REGISTER(shallow::H, shallow::G);

namespace balanced {
    struct B : A { KCL_RTTI_IMPL(); B() { priori(this); } };
    struct C : B { KCL_RTTI_IMPL(); C() { priori(this); } };
    struct D : B { KCL_RTTI_IMPL(); D() { priori(this); } };

    struct E : A { KCL_RTTI_IMPL(); E() { priori(this); } };
    struct F : E { KCL_RTTI_IMPL(); F() { priori(this); } };
    struct G : E { KCL_RTTI_IMPL(); G() { priori(this); } };
    struct H : E { KCL_RTTI_IMPL(); H() { priori(this); } };
}
KCL_RTTI_REGISTER(balanced::B, A);
KCL_RTTI_REGISTER(balanced::C, balanced::B);
KCL_RTTI_REGISTER(balanced::D, balanced::B);
KCL_RTTI_REGISTER(balanced::E, A);
KCL_RTTI_REGISTER(balanced::F, balanced::E);
KCL_RTTI_REGISTER(balanced::G, balanced::E);
KCL_RTTI_REGISTER(balanced::H, balanced::E);

// Same interface as A, but not related.
struct Z {
    KCL_RTTI_IMPL();
    uint64_t get() { return 1; };
};
KCL_RTTI_REGISTER(Z);


void draw_bar(float percent, std::string s = "-") {
    const auto cols = 60;
    uint64_t width = cols * percent * 4.0;
    if (width > cols) {
        width = cols;
        printf("|");
        while (width--) std::cout << s;
        printf("...\n");
    } else {
        printf("|");
        while (width--) std::cout << s;
        printf("|\n");
    }
}

uint64_t run(std::string label, std::function<uint64_t()> benchmark)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    auto successes = benchmark();
    auto t2 = std::chrono::high_resolution_clock::now();

    auto usecs_per_iterations =
        std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
    auto num_ops = num_usecs_per_sec / (float(usecs_per_iterations) / n);
    if (max_num_ops == 0) max_num_ops = num_ops; // the first run will be 100%
    auto percent = float(num_ops) / float(max_num_ops);
    printf(
            "%3s: %5.1f MHz (%3.0f%%) [%7lu] ",
            label.c_str(),
            num_ops / num_usecs_per_sec,
            percent * 100, successes
          );
    draw_bar(percent);
    return num_ops;
}

void generate_data(std::vector<std::shared_ptr<A>>& v, Hierarchy h, unsigned int from = 7, unsigned int width = 0)
{
    v.reserve(n); // ensure contiguous memory
    for(uint64_t i = 0; i < n; i++) {
        uint64_t val = from + rand() % (width + 1);
        if (h == Hierarchy::deep) {
            switch(val) {
                case  0: v.emplace_back(std::make_shared<A>()); break;
                case  1: v.emplace_back(std::make_shared<deep::B>()); break;
                case  2: v.emplace_back(std::make_shared<deep::C>()); break;
                case  3: v.emplace_back(std::make_shared<deep::D>()); break;
                case  4: v.emplace_back(std::make_shared<deep::E>()); break;
                case  5: v.emplace_back(std::make_shared<deep::F>()); break;
                case  6: v.emplace_back(std::make_shared<deep::G>()); break;
                case  7: v.emplace_back(std::make_shared<deep::H>()); break;
            }
        } else if (h == Hierarchy::shallow) {
            switch(val) {
                case  0: v.emplace_back(std::make_shared<A>()); break;
                case  1: v.emplace_back(std::make_shared<shallow::B>()); break;
                case  2: v.emplace_back(std::make_shared<shallow::C>()); break;
                case  3: v.emplace_back(std::make_shared<shallow::D>()); break;
                case  4: v.emplace_back(std::make_shared<shallow::E>()); break;
                case  5: v.emplace_back(std::make_shared<shallow::F>()); break;
                case  6: v.emplace_back(std::make_shared<shallow::G>()); break;
                case  7: v.emplace_back(std::make_shared<shallow::H>()); break;
            }
        } else if (h == Hierarchy::balanced) {
            switch(val) {
                case  0: v.emplace_back(std::make_shared<A>()); break;
                case  1: v.emplace_back(std::make_shared<balanced::B>()); break;
                case  2: v.emplace_back(std::make_shared<balanced::C>()); break;
                case  3: v.emplace_back(std::make_shared<balanced::D>()); break;
                case  4: v.emplace_back(std::make_shared<balanced::E>()); break;
                case  5: v.emplace_back(std::make_shared<balanced::F>()); break;
                case  6: v.emplace_back(std::make_shared<balanced::G>()); break;
                case  7: v.emplace_back(std::make_shared<balanced::H>()); break;
            }
        }
    }
}

void shuffle(std::vector<std::shared_ptr<A>>& v) {
    auto rng = std::default_random_engine { };
    std::shuffle(std::begin(v), std::end(v), rng);
}

void print_average(float num) {
    auto avg = num / 9.0;
    printf("------------\n");
    printf("AVG: %5.1f MHz                  ", avg / num_usecs_per_sec);
    draw_bar(avg / max_num_ops, "=");
}

float dummy = 0;
void run_benchmarks(std::vector<std::shared_ptr<A>>& v, Hierarchy h)
{
    float sum = 0;

    // Cache warming
    dummy += [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = static_cast <      A*>(e.get()); p ? s++ : dummy++; } return s; }();

    printf("Base-line: static_cast\n");
    printf("```\n");
    dummy += run("-", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = static_cast <      A*>(e.get()); p ? s++ : dummy++; } return s; });
    printf("```\n\n");

    if (h == Hierarchy::deep) {
        printf("Implementation: `dynamic_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<      A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<deep::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<deep::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<deep::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<deep::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<deep::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<deep::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<deep::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<      Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n");

        printf("Implementation: `priori_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<      A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<deep::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<deep::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<deep::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<deep::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<deep::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<deep::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<deep::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<      Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n\n");

        printf("Implementation: `kcl_dynamic_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<      A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<deep::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<deep::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<deep::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<deep::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<deep::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<deep::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<deep::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<      Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n\n");

    } else if (h == Hierarchy::shallow) {
        printf("Implementation: `dynamic_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<         A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<shallow::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<shallow::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<shallow::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<shallow::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<shallow::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<shallow::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<shallow::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<         Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n\n");

        printf("Implementation: `priori_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<         A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<shallow::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<shallow::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<shallow::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<shallow::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<shallow::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<shallow::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<shallow::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<         Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n\n");

        printf("Implementation: `kcl_dynamic_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<         A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<shallow::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<shallow::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<shallow::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<shallow::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<shallow::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<shallow::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<shallow::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<         Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n\n");

    } else if (h == Hierarchy::balanced) {
        printf("Implementation: `dynamic_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<          A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<balanced::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<balanced::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<balanced::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<balanced::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<balanced::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<balanced::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<balanced::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = dynamic_cast<          Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n\n");

        printf("Implementation: `priori_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<          A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<balanced::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<balanced::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<balanced::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<balanced::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<balanced::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<balanced::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<balanced::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = priori_cast<          Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n\n");

        printf("Implementation: `kcl_dynamic_cast`\n");
        printf("```\n");
        sum = 0;
        dummy += run("A", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<          A*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("B", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<balanced::B*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("C", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<balanced::C*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("D", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<balanced::D*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("E", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<balanced::E*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("F", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<balanced::F*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("G", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<balanced::G*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("H", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<balanced::H*>(e.get()); p ? s++ : dummy++; } return s; });
        sum   += run("Z", [&v]() -> uint64_t { auto s = 0; for (auto& e: v) { auto *p = kcl_dynamic_cast<          Z*>(e.get()); p ? s++ : dummy++; } return s; });
        print_average(sum);
        printf("```\n\n");
    }
}

struct JustKclRtti {
    KCL_RTTI_IMPL();
};
KCL_RTTI_REGISTER(JustKclRtti);

std::vector<std::shared_ptr<A>> vec_deep_successful;
std::vector<std::shared_ptr<A>> vec_deep_fails;

std::vector<std::shared_ptr<A>> vec_shallow_successful;
std::vector<std::shared_ptr<A>> vec_shallow_fails;

std::vector<std::shared_ptr<A>> vec_deep_mixed;
std::vector<std::shared_ptr<A>> vec_shallow_mixed;
std::vector<std::shared_ptr<A>> vec_balanced_mixed;

int main()
{
    generate_data(vec_deep_successful, Hierarchy::deep, 6, 0);
    generate_data(vec_deep_fails, Hierarchy::deep, 1, 0);
    generate_data(vec_deep_mixed, Hierarchy::deep, 0, 6);

    generate_data(vec_shallow_successful, Hierarchy::shallow, 6, 0);
    generate_data(vec_shallow_fails, Hierarchy::shallow, 1, 0);
    generate_data(vec_shallow_mixed, Hierarchy::shallow, 0, 6);

    generate_data(vec_balanced_mixed, Hierarchy::balanced, 0, 6);

    // Run the benchmark loop 3 times:
    // 1st: Warming up, discard.
    // 2nd: Objects are ordered in memory
    // 3rd: Objects are shuffled in memory
    for (unsigned int i = 0; i < 3; i++) {
        max_num_ops = 0;

        printf("\n\n\n\n\n");

        switch(i) {
            case 0:
                printf("## Run 0 (discard)\n\n");
                break;
            case 1:
                printf("## Run 1 (objects aligned)\n\n");
                break;
            case 2:
                printf("## Run 2 (objects shuffled)\n\n");

                shuffle(vec_deep_successful);
                shuffle(vec_deep_fails);
                shuffle(vec_deep_mixed);
                shuffle(vec_shallow_successful);
                shuffle(vec_shallow_fails);
                shuffle(vec_shallow_mixed);
                shuffle(vec_balanced_mixed);
        }

        printf("### Class hierarchy: deep\n\n");

        printf("#### Cast type: Mostly successful (cast from class G)\n\n");
        run_benchmarks(vec_deep_successful, Hierarchy::deep);

        printf("#### Cast type: Mostly failed (cast from class B)\n\n");
        run_benchmarks(vec_deep_fails, Hierarchy::deep);

        printf("#### Cast type: Mixed (cast from random classes)\n\n");
        run_benchmarks(vec_deep_mixed, Hierarchy::deep);


        printf("\n\n\n\n\n");
        printf("### Class hierarchy: shallow\n\n");

        printf("#### Cast type: Mostly successful (cast from class G)\n\n");
        run_benchmarks(vec_shallow_successful, Hierarchy::shallow);

        printf("#### Cast type: Mostly failed (cast from class B)\n\n");
        run_benchmarks(vec_shallow_fails, Hierarchy::shallow);

        printf("#### Cast type: Mixed (cast from random classes)\n\n");
        run_benchmarks(vec_shallow_mixed, Hierarchy::shallow);


        printf("\n\n\n\n\n");
        printf("### Class hierarchy: balanced\n\n");

        printf("#### Cast type: Mixed (cast from random classes)\n\n");
        run_benchmarks(vec_balanced_mixed, Hierarchy::balanced);
    }

    printf("\n\n\n\n\n");
    std::cout << "sizeof JustKclRtti: " << sizeof(JustKclRtti) << "\n";
    std::cout << "sizeof A: " << sizeof(A) << "\n";
    printf("%f", dummy);
}
