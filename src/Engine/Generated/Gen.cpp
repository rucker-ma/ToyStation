#include "Gen.h"
#include "Base/Calculagraph.h"
BEGIN_DEFINE(Calculagraph)
.AddVariable("OnEnd", &Calculagraph::OnEnd)
.AddVariable("name_", &Calculagraph::name_)
.AddVariable("previous_time_", &Calculagraph::previous_time_)
.AddVariable("count_", &Calculagraph::count_)
.AddVariable("interval_", &Calculagraph::interval_)
.AddVariable("multiply_", &Calculagraph::multiply_)
.AddVariable("result_", &Calculagraph::result_)
.AddFunction("Start", &Calculagraph::Start)
.AddFunction("Reset", &Calculagraph::Reset)
.AddFunction("Step", &Calculagraph::Step)
END_DEFINE()