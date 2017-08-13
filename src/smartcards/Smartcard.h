//
// Created by david on 8/11/17.
//

#ifndef WINSCARD_STUB_SMARTCARD_H
#define WINSCARD_STUB_SMARTCARD_H

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <pcsclite.h>

namespace smartcards {

  /**
   * Smartcard virtual simulator base class. The specific implemenations wlll have to override the execute
   * function
   */
  class Smartcard {

  public:
    /**
     * Constructor which will define the behavior of sharing mode and the supported protocols
     * Must be called by the derived class
     * @param dwSharingMode
     * @param dwProtocol
     */
    explicit Smartcard(const std::vector<unsigned  char> &atr, DWORD dwSharingMode = SCARD_SHARE_SHARED, DWORD dwProtocol = SCARD_PROTOCOL_T0);

    /**
     * Connect to the smartcard, which will verify the supported sharingMode and protocols. The connect function returns
     * a handle to the smartcard and the active protocol.
     *
     * @param dwShareMode
     * @param dwPreferredProtocols
     * @param phCard
     * @param pdwActiveProtocol
     * @return SCARD_S_SUCCESS, SCARD_E_INVALID_VALUE
     */
    DWORD connect(DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol);

    /**
     * Disconnect from the smartcard with extra action to take for the card during the disconnect
     *
     * @param handle The handle of the Smartcard to disconnect
     * @param disposition action to take of the smartcard
     * @return SCARD_S_SUCCESS, SCARD_E_INVALID_HANDLE
     */
    DWORD disconnect(SCARDHANDLE handle, DWORD dwDisposition);

    /**
     * Begin multiple calls to the Smartcard
     * @param handle The handle to the smartcard
     * @return SCARD_S_SUCCESS, SCARD_E_SHARING_VIOLATION, SCARD_E_INVALID_HANDLE
     */
    DWORD beginTransaction(SCARDHANDLE handle);

    /**
     * End multiple calls to the Smartcard
     * @param handle The handle to the smartcard
     * @return SCARD_S_SUCCESS, SCARD_E_SHARING_VIOLATION, SCARD_E_INVALID_HANDLE
     */
    DWORD endTransaction(SCARDHANDLE handle, DWORD dwDisposition);

    /**
     * Function to override by the specific implemented Smartcard.
     *
     * @param scardhandle handle to the smartcard
     * @param in_apdu APDU command
     * @param in_apdu_lg length of the APDU command
     * @param out_apdu APDU result
     * @param out_apd_lg length of the APDU result
     * @return SCARD_S_SUCCESS
     */
    virtual DWORD execute(SCARDHANDLE handle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) = 0;

    DWORD getPreferredProtocol();

    /**
     * Function to override which returns the ATR pointer
     */
    const std::vector<unsigned char> getATR();

    /**
     * Static member function for the factory method to instantiate the supported Smartcards.
     * @param impl
     * @return smart pointer to the SmartCard object
     */
    static std::unique_ptr<Smartcard> instance_of(const std::string &impl);

  private:

    struct SmartCardContext {
      DWORD sharingMode;
      DWORD protocol;
      bool  transaction;
    };

    SCARDHANDLE scardHandleIndex{0};
    std::unordered_map<SCARDHANDLE, std::unique_ptr<SmartCardContext>> scardHandles;
    DWORD allowedSharingModes;
    DWORD allowedProtocol;
    DWORD disposition;
    std::vector<unsigned char> ATR;
  };

}


#endif //WINSCARD_STUB_SMARTCARD_H
