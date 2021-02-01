// Sending zero ether to a contract should still invoke the receive ether function
// (it previously did not because the gas stipend was not provided by the EVM)
contract Receiver {
    receive() external payable {}
}


contract Main {
    constructor() payable {}

    function s() public returns (bool) {
        Receiver r = new Receiver();
        return payable(r).send(0);
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// constructor(), 20 wei ->
// gas ir: 216460
// gas irOptimized: 123804
// gas legacy: 153937
// gas legacyOptimized: 140425
// s() -> true
