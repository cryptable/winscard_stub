//
// Created by david on 8/11/17.
//

#ifndef WINSCARD_STUB_SMARTCARDREADER_H
#define WINSCARD_STUB_SMARTCARDREADER_H

#include <string>
#include <memory>
#include <pcsclite.h>
#include "../smartcards/Smartcard.h"
#include "../eventhandler/SmartcardEventSubject.h"

namespace readers {

  /**
   * Smartcard reader simulator: This class will be the base class for the reader implementation. In a reader only 1 card
   * can be inserted. All calls from the winscard interface will  be proxied through the reader implementation.
   */
  class SmartcardReader {

  public:

    SmartcardReader() = delete;

    SmartcardReader(SmartcardReader &other) = delete;

    SmartcardReader &operator=(SmartcardReader &other) = delete;

    SmartcardReader(SmartcardReader &&other) = delete;

    SmartcardReader &operator=(SmartcardReader &&other) = delete;

    ~SmartcardReader() = default;

    /**
     * Constructor of Smartcard to be called by the derived class
     *
     * @param readerName
     */
    explicit SmartcardReader(std::string readerName);

    /**
     * Returns the default name of the reader and will be used by 'winscard' create a unique reader name
     *
     * @return the name of the reader
     */
    std::string getName() const ;

    /**
     * Proxy function to connect to the Smartcard all parameters are send to the smartcard.
     *
     * @param dwShareMode
     * @param dwPreferredProtocols
     * @param phCard
     * @param pdwActiveProtocol
     * @return SCARD_S_SUCCESS, SCARD_E_NO_SMARTCARD
     */
    DWORD connectToSmartCard(DWORD dwShareMode, DWORD dwPreferredProtocols, LPDWORD pdwActiveProtocol);

    /**
     * Proxy function to disconnect from the Smartcard all parameters are send to the smartcard.
     *
     * @param dwShareMode (see SmartCard base class)
     * @param dwPreferredProtocols (see SmartCard base class)
     * @param phCard (see SmartCard base class)
     * @param pdwActiveProtocol (see SmartCard base class)
     * @return SCARD_S_SUCCESS, SCARD_E_NO_SMARTCARD
     */
    DWORD disconnectFromSmartCard(DWORD dwDisposition);

    /**
     * Function which inserts a Smartcard into the reader
     *
     * @param card name of one of the implemented cards ("test")
     * @return SCARD_S_SUCCESS, SCARD_E_CARD_IN_READER, SCARD_E_CARD_UNSUPPORTED
     */
    DWORD insertCard(const std::string &card);

    /**
     * Function which removes a Smartcard from the reader
     *
     * @param card name of one of the implemented cards ("test")
     * @return SCARD_S_SUCCESS, SCARD_E_CARD_IN_READER, SCARD_E_CARD_UNSUPPORTED
     */
    DWORD ejectCard();

    /**
     * Proxy begin transaction to the smartcard
     *
     * @param scardhandle handle to the scmartcard
     * @return SCARD_S_SUCCESS, SCARD_E_NO_SMARTCARD + errors of Smartcard
     */
    DWORD beginTransactionOnSmartcard();

    /**
     * Proxy end transaction to the smartcard if SCARD_EJECT_CARD is send as dwDisposition, the smartcard is removed
     *
     * @param scardhandle handle to the scmartcard
     * @return SCARD_S_SUCCESS, SCARD_E_NO_SMARTCARD + errors of Smartcard
     */
    DWORD endTransactionOnSmartcard(DWORD dwDisposition);

    /**
     * Get the status of the smartcard inserted in the reader
     * @param hCard handle of the card
     * @param mszReaderName name of the reader
     * @param pcchReaderLen length of the reader's name
     * @param pdwState state of the smartcard
     * @param pdwProtocol protocol used with the smartcard
     * @param pbAtr ATR of the smartcard
     * @param pcbAtrLen length of the ATR
     * @return
     */
    DWORD smartcardStatus(LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen);

    /**
     * The function which must be overwritten by the implemented reader, which should be mainly a pinpad
     * or non-pinpad reader. It will -most of the time- be a proxy function to the smartcard.
     *
     * @param scardhandle handle to the smartcard
     * @param in_apdu APDU command
     * @param in_apdu_lg length of the APDU command
     * @param out_apdu APDU result
     * @param out_apd_lg length of the APDU result
     * @return SCARD_S_SUCCESS,
     */
    virtual DWORD execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) = 0;

    /**
     * Function to instantiate a supported reader ("Non Pinpad Reader", "Pinpad Reader")
     *
     * @param impl name of the supported reader
     * @return smart pointer to the SmartCardReader object
     */
    static std::shared_ptr<SmartcardReader> instance_of(const std::string &impl);

    /**
     * SetId: number for duplicate reader
     * TODO: split into context and template
     */
    void setId(unsigned int nbr);

    /**
     * Function
     * @return return the ID of the reader
     */
    const std::string &getReaderIdentifier();

    /**
     * Function to retrieve information for the readerState
     * @param readerState
     */
    void getEventInfo(LPSCARD_READERSTATE readerState);

    /**
     * Function to attach to the smartcard event handler of the reader
     * @param observer
     */
    void attachSmartcardEvents(std::shared_ptr<eventhandler::WinscardEventObserver> observer);

    /**
     * Function to deattach of the smartcard event handler of the reader
     * @param observer
     */
    void deattachSmartcardEvents(std::shared_ptr<eventhandler::WinscardEventObserver> &observer);

  protected:
    const std::string name;

  private:

    DWORD getState();

    std::unique_ptr<smartcards::Smartcard> smartcard;

    unsigned int events;

    unsigned int id;

    std::string readerId;

    eventhandler::SmartcardEventSubject smartcardEvents;
  };

}

#endif //WINSCARD_STUB_SMARTCARDREADER_H
