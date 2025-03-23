
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
    WaitWiFiConnection,
    RespondWaiting,
    SuccessRespond,
    WrongPayload,
    FailRespond,
};

};