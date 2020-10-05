/*
 * GeFaST
 *
 * Copyright (C) 2016 - 2020 Robert Mueller
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: Robert Mueller <romueller@techfak.uni-bielefeld.de>
 * Faculty of Technology, Bielefeld University,
 * PO box 100131, DE-33501 Bielefeld, Germany
 */

#include <fstream>

#include "../../include/modes/DereplicationMode.hpp"

#include "../../include/Factories.hpp"

namespace GeFaST {

    DereplicationConfiguration::DereplicationConfiguration(int argc, const char* argv[]) {

        set_general_parameters(argc, argv);

        /* Set default values */

        main_threshold = 0;
        refinement_threshold = 0;


        /* Read mode-specific configuration from file */

        std::ifstream in_stream(configuration_file);
        std::string line;
        unsigned long delim_pos;

        while (std::getline(in_stream, line).good()) {

            if (line.empty() || line.front() == '#') continue;

            delim_pos = line.find('=');
            std::string key = line.substr(0, delim_pos);

            // currently nothing to do

        }

        check();


        /* mode-specific data structures and settings */

        // opt_preprocessor: user-defined, handled in set_general_parameters()
        // opt_quality_encoding: user-defined, handled in set_general_parameters()
        // opt_amplicon_collection: given by opt_amplicon_storage

        opt_distance = DT_BOUNDED_LEVENSHTEIN;
        opt_clusterer = CL_DEREPLICATOR;
        opt_refiner = CR_IDLE;
        opt_output_generator = OG_DEREPLICATION;

        opt_amplicon_storage = (opt_preprocessor == PP_FASTQ) ? AS_PREPARED_LENGTH_POOLS_QUALITY : AS_PREPARED_LENGTH_POOLS;
        opt_swarm_storage = SS_SIMPLE_PER_POOL;
        opt_auxiliary_data = AD_NAIVE_AUXILIARY;
        opt_refinement_auxiliary_data = RD_NAIVE_AUXILIARY;

    }

    DereplicationConfiguration* DereplicationConfiguration::clone() const {
        return new DereplicationConfiguration(*this);
    }


    Preprocessor* DereplicationConfiguration::build_preprocessor() const {
        return PreprocessorFactory::create(opt_preprocessor, opt_quality_encoding, *this);
    }

    QualityEncoding<>* DereplicationConfiguration::build_quality_encoding() const {
        return new QualityEncoding<>(opt_quality_encoding);
    }

    Clusterer* DereplicationConfiguration::build_clusterer() const {
        return ClustererFactory::create(opt_clusterer, *this);
    }

    ClusterRefiner* DereplicationConfiguration::build_cluster_refiner() const {
        return ClusterRefinerFactory::create(opt_refiner, *this);
    }

    OutputGenerator* DereplicationConfiguration::build_output_generator() const {
        return OutputGeneratorFactory::create(opt_output_generator, *this);
    }

    AmpliconStorage* DereplicationConfiguration::build_amplicon_storage(const DataStatistics<>& ds) const {
        return AmpliconStorageFactory::create(opt_amplicon_storage, ds, *this);
    }

    SwarmStorage* DereplicationConfiguration::build_swarm_storage(const AmpliconStorage& amplicon_storage) const {
        return SwarmStorageFactory::create(opt_swarm_storage, amplicon_storage, *this);
    }

    AuxiliaryData* DereplicationConfiguration::build_auxiliary_data(const AmpliconStorage& amplicon_storage,
            const numSeqs_t pool_id, const dist_t threshold) const {
        return AuxiliaryDataFactory::create(opt_auxiliary_data, amplicon_storage, pool_id, threshold, *this);
    }

    AuxiliaryData* DereplicationConfiguration::build_auxiliary_data(const AmpliconStorage& amplicon_storage,
            const SwarmStorage& swarm_storage, const numSeqs_t pool_id, const dist_t threshold) const {

        return AuxiliaryDataFactory::create(opt_refinement_auxiliary_data, amplicon_storage, swarm_storage,
                pool_id, threshold, *this);

    }

    Distance* DereplicationConfiguration::build_distance_function(const AmpliconStorage &amplicon_storage,
                                                                  const dist_t threshold) const {
        return DistanceFactory::create(opt_distance, amplicon_storage, threshold, *this);
    }


    void DereplicationConfiguration::print(std::ostream& stream) const {
        Configuration::print(stream);
    }


    void DereplicationConfiguration::check() const {

        std::string mode = get_configuration_label(CF_DEREPLICATION);
        std::set<PreprocessorOption> valid_pp = {PP_FASTA, PP_FASTQ};
        std::set<ClustererOption > valid_cl = {CL_IDLE};
        std::set<ClusterRefinerOption > valid_cr = {CR_IDLE};
        std::set<OutputGeneratorOption> valid_og = {OG_IDLE};
        std::set<QualityEncodingOption> valid_qe = {QE_SANGER, QE_ILLUMINA13, QE_ILLUMINA15, QE_ILLUMINA18};
//        std::set<DistanceOption> valid_dt = {};

        if (valid_pp.find(opt_preprocessor) == valid_pp.end()){
            std::cerr << "ERROR: Mode '" << mode << "' supports only the FASTA and FASTQ preprocessors." << std::endl;
            abort();
        }

        if (valid_cl.find(opt_clusterer) == valid_cl.end()) {
            std::cerr << "WARNING: Mode '" << mode << "' ignores changes of the 'clusterer' option." << std::endl;
        }

        if (valid_cr.find(opt_refiner) == valid_cr.end()) {
            std::cerr << "WARNING: Mode '" << mode << "' ignores changes of the 'refiner' option." << std::endl;
        }

        if (valid_og.find(opt_output_generator) == valid_og.end()) {
            std::cerr << "WARNING: Mode '" << mode << "' ignores changes of the 'output_generator' option." << std::endl;
        }

        if (opt_preprocessor == PP_FASTQ && valid_qe.find(opt_quality_encoding) == valid_qe.end()) {
            std::cerr << "ERROR: Invalid 'quality_encoding' option for mode '" << mode << "'." << std::endl;
            abort();
        }

//        if (valid_dt.find(opt_distance) == valid_dt.end()) {
//            std::cerr << "WARNING: Mode '' ..." << std::endl;
//            std::cerr << "ERROR: Mode '' ..." << std::endl;
//            abort();
//        }

    }

}