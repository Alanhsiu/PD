For Evaluator:
    Usage: 
    ./evaluator.sh [input.block] [input.net] [output.rpt] [alpha]
    ## Note that we only support alpha=0.5 for public evaluation ##

    Ex: bash evaluator.sh ami33.block ami33.nets ami33.rpt 0.5
    Results: 
    [Eval] Run cmd: ./checker ami33.block ami33.nets ami33.rpt 0.5

    Checking Report...
    cost:       actual  674155.500000/reported 674155.000000
    wirelength: actual  94989.000000/reported 94989.000000
    area:       actual  1253322.000000/reported 1253322.000000
    width:      outline 1326/actual 1218/reported 1218.000000
    height:     outline 1205/actual 1029/reported 1029.000000
    runtime: 0.055697

    Congradulations, your results are correct

    [Eval] Your area & wirelength: 1253322.000000 & 94989.000000
    [Eval] Your runtime: 0.055697
    [Eval] Your normalized cost = 0.5*(A-minA)/(maxA-minA) + 0.5*(WL-minWL)/(maxWL-minWL) = 0.032327
    [Eval] SCORE = QUALITY_SCORE*0.8 + RUNTIME_SCORE*0.2
                = 8.5230219*0.8 + 8.105104*0.2
                = 8.43944

    --
    Contact: Cheng-Yu Chiang <frankchiang@eda.ee.ntu.edu.tw>


For Checker:
	Usage:
	./checker [input block] [input net] [output] [alpha]

    Ex: ./checker ami33.block ami33.nets ami33.rpt 0.5

    Results:
	Checking Report...
	cost:       actual  626661.000000/reported 674155.000000
	wirelength: actual  0.000000/reported 94989.000000
	area:       actual  1253322.000000/reported 1253322.000000
	width:      outline 1326/actual 1218/reported 1218.000000
	height:     outline 1205/actual 1029/reported 1029.000000
	runtime: 0.055697
	
	Congradulations, your results are correct
	--
