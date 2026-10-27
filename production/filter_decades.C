// =============================================================================
// filter_decades.C  —  split one HepMC3 ROOT tree into three Bjorken-x decades
// =============================================================================
//
// Makes the 1.1.0 samples from 1.0.0 (see README.md, "1.1.0"). Reads the input
// once and writes every event with x in [1e-4,1e-3), [1e-3,1e-2) or [1e-2,1e-1)
// (event attribute "x") unchanged to the matching output. Events outside
// [1e-4,1e-1) are dropped. The input may be an XRootD URL.
//
//   root -l -b -q 'filter_decades.C+("<in>.hepmc3.tree.root","<out pattern>")'
//
// <out pattern> is the output file name with "{x}" where the decade tag goes
// (x1e-4to1e-3, x1e-3to1e-2, x1e-2to1e-1). The last three lines printed give
// the events kept per decade.
//
// =============================================================================

R__LOAD_LIBRARY(libHepMC3)
R__LOAD_LIBRARY(libHepMC3rootIO)

#include "HepMC3/ReaderRootTree.h"
#include "HepMC3/WriterRootTree.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/Attribute.h"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

using namespace HepMC3;

static double attrD(const GenEvent& ev, const char* name, double def = 0.)
{
    auto ad = ev.attribute<DoubleAttribute>(name);
    if (ad) return ad->value();
    auto as = ev.attribute<StringAttribute>(name);
    if (as) return atof(as->value().c_str());
    return def;
}

void filter_decades(const char* in_path, const char* out_pattern)
{
    const double edges[4] = {1e-4, 1e-3, 1e-2, 1e-1};
    const char* tags[3] = {"x1e-4to1e-3", "x1e-3to1e-2", "x1e-2to1e-1"};
    const std::string pattern(out_pattern);
    const size_t at = pattern.find("{x}");
    if (at == std::string::npos) { fprintf(stderr, "ERROR: no {x} in %s\n", out_pattern); exit(1); }

    ReaderRootTree reader(in_path);
    if (reader.failed()) { fprintf(stderr, "ERROR: cannot open %s\n", in_path); exit(1); }
    std::unique_ptr<WriterRootTree> writer[3];
    for (int d = 0; d < 3; d++) {
        std::string out = pattern;
        out.replace(at, 3, tags[d]);
        writer[d].reset(new WriterRootTree(out, reader.run_info()));
        if (writer[d]->failed()) { fprintf(stderr, "ERROR: cannot create %s\n", out.c_str()); exit(1); }
    }

    long n_read = 0, n_kept[3] = {0, 0, 0};
    while (!reader.failed()) {
        GenEvent evt(Units::GEV, Units::MM);
        reader.read_event(evt);
        if (reader.failed()) break;
        n_read++;
        if (n_read % 1000000 == 0)
            printf("  ... %ld events read, kept %ld / %ld / %ld\n",
                   n_read, n_kept[0], n_kept[1], n_kept[2]);

        double x = attrD(evt, "x", -1.);
        if (x < 0) { fprintf(stderr, "ERROR: event %ld has no x attribute\n", n_read); exit(1); }
        for (int d = 0; d < 3; d++)
            if (x >= edges[d] && x < edges[d + 1]) { writer[d]->write_event(evt); n_kept[d]++; break; }
    }
    reader.close();
    for (auto& w : writer) w->close();

    printf("Input : %s\n", in_path);
    for (int d = 0; d < 3; d++)
        printf("x in [%g, %g): kept %ld of %ld events read (%.3f%%)\n", edges[d], edges[d + 1],
               n_kept[d], n_read, 100.0 * n_kept[d] / (n_read > 0 ? n_read : 1));
}
