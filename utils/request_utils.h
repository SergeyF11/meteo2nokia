
#pragma once

namespace AsyncRequest {

enum Error
{
    OK,
    NoConnection,
    SendedAlready,
    WrongRequest,
    //NoResponse,
    // NoData,
    // ErrorData,
};
    
enum State {
    Unknown,
    RespondWaiting,
    SuccessRespond,
    WrongPayload,
    FailRespond,
};

};