// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "google/fhir/json_format.h"
#include "google/fhir/r4/profiles.h"
#include "proto/r4/core/resources/patient.pb.h"
#include "proto/myprofile/myprofile.pb.h"
#include "cc/example_utils.h"

using std::string;

using ::google::fhir::PrintFhirToJsonStringForAnalytics;
using ::fhirexamples::myprofile::DemoPatient;

double Rand() {
  static std::default_random_engine generator;
  static std::uniform_real_distribution<> distribution(0, 1);
  return distribution(generator);
}

// This is a comprehensive example that is meant to be run last of the C++
// examples!
// This example involves the full process of generating a custom profile with
// custom extensions, and ultimately uploading them to BigQuery.
// For instructions on setting up your workspace, see the top-level README.md
//
// To generate the myprofile.proto, run
// generate_definitions_and_protos.sh //proto/myprofile:myprofile
//
// To run:
// bazel build //cc:ProfilePatientsToCustomProfile
// bazel-bin/cc/ProfilePatientsToCustomProfile $WORKSPACE
//
// This generates $WORKSPACE/analytic/DemoPatient.analytic.ndjson
//
// Next, generate the analytic schema for demo patient:
// bazel build //java:GenerateBigQuerySchema.java $WORKSPACE
//
// Finally, upload the DemoPatients to BigQuery:
// shell/upload_demo_patients.sh
//
// Now, you can run queries like
// SELECT favorites, likesPie FROM `$PROJECT.fhirexamples.DemoPatient` LIMIT 10

int main(int argc, char** argv) {
  const std::string workspace = argv[1];

  absl::TimeZone time_zone;
  CHECK(absl::LoadTimeZone("America/Los_Angeles", &time_zone));

  std::vector<DemoPatient> patients =
      fhir_examples::ReadNdJsonFile<DemoPatient>(
          time_zone, absl::StrCat(workspace, "/ndjson/Patient.fhir.ndjson"));

  std::ofstream write_stream;
  write_stream.open(
      absl::StrCat(workspace, "/analytic/DemoPatient.analytic.ndjson"));


  for (DemoPatient& patient : patients) {
    if (Rand() > .15) {
      patient.mutable_likes_pie()->set_value(true);
    }

    patient.mutable_favorites()
           ->mutable_favorite_number()
           ->set_value((int) (Rand() * 100));

    if (Rand() > .6) {
      patient.mutable_favorites()
             ->mutable_pet_names()
             ->mutable_dog()
             ->set_value("Fido");
    } else {
      patient.mutable_favorites()
             ->mutable_pet_names()
             ->mutable_dog()
             ->set_value("Spot");
    }

    if (Rand() > .6) {
      patient.mutable_favorites()
             ->mutable_pet_names()
             ->mutable_cat()
             ->set_value("Pippen");
    } else {
      patient.mutable_favorites()
             ->mutable_pet_names()
             ->mutable_cat()
             ->set_value("Ivan");
    }

    write_stream <<
        PrintFhirToJsonStringForAnalytics(patient).ValueOrDie();
    write_stream << "\n";
  }
  write_stream.close();
}
