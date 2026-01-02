#pragma once

#include "interface/IMLGuiView.h"
#include "interface/IMLModelObserver.h"

class wxMLMainFrame;

class FMLGuiView : public IMLGuiView, public IMLModelObserver
{
public:
    FMLGuiView();
    virtual ~FMLGuiView() override;
    
public:
    // View Interface
    
    // ModelObserver Interface
    // Transaction Events
    void OnTransactionAdded(const FMLTransactionData& transactionData) override;
    void OnTransactionRemoved(const int transactionId) override;
    void OnTransactionUpdated(const FMLTransactionData& transactionData) override;
    
    // Data Events
    void OnTransactionsCleared() override;
    void OnDataLoaded() override;
    void OnDataSaved() override;
    
private:
    wxMLMainFrame* MainFrame = nullptr;
};
