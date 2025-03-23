
#pragma once

namespace AsyncRequest {

enum Error
{
    OK,
    WaitConnection,
    NoConnection,
    SendedAlready,
    WrongRequest,
    //NoResponse,
    // NoData,
    // ErrorData,
};
    
enum State {
    Unknown,
    ConnectionWaiting,
    RespondWaiting,
    SuccessRespond,
    WrongPayload,
    FailRespond,
};

};