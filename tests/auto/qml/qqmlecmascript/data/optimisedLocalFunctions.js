var variable
variable = "variable"

var variable2 = "variable2"

function qml_global_function () {
}

function func() {
    function local_function () {
    }
    // high enough to get it to opt; 10000 seems to be too low
    for (var i = 0; i < 100000; ++i) qml_global_function()
    for (var i = 0; i < 100000; ++i) local_function()
    return "func"
}
