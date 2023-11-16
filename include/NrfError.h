#pragma once

namespace bat {
char const *nrf_error_message(int error) {
  switch (error) {
    default:
      return "Unknown error.";

    case NRF_SUCCESS:
      return "Successful command.";

    case NRF_ERROR_SVC_HANDLER_MISSING:
      return "SVC handler is missing.";

    case NRF_ERROR_SOFTDEVICE_NOT_ENABLED:
      return "SoftDevice has not been enabled.";

    case NRF_ERROR_INTERNAL:
      return "Internal Error.";

    case NRF_ERROR_NO_MEM:
      return "No Memory for operation.";

    case NRF_ERROR_NOT_FOUND:
      return "Not found.";

    case NRF_ERROR_NOT_SUPPORTED:
      return "Not supported.";

    case NRF_ERROR_INVALID_PARAM:
      return "Invalid Parameter.";

    case NRF_ERROR_INVALID_STATE:
      return "Invalid state, operation disallowed in this state.";

    case NRF_ERROR_INVALID_LENGTH:
      return "Invalid Length.";

    case NRF_ERROR_INVALID_FLAGS:
      return "Invalid Flags.";

    case NRF_ERROR_INVALID_DATA:
      return "Invalid Data.";

    case NRF_ERROR_DATA_SIZE:
      return "Invalid Data size.";

    case NRF_ERROR_TIMEOUT:
      return "Operation timed out.";

    case NRF_ERROR_NULL:
      return "Null Pointer.";

    case NRF_ERROR_FORBIDDEN:
      return "Forbidden Operation.";

    case NRF_ERROR_INVALID_ADDR:
      return "Bad Memory Address.";

    case NRF_ERROR_BUSY:
      return "Busy.";

    case NRF_ERROR_CONN_COUNT:
      return "Maximum connection count exceeded.";

    case NRF_ERROR_RESOURCES:
      return "Not enough resources for operation.";
  }
}
}  // namespace bat
