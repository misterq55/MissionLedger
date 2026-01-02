#include "MLGuiView.h"

FMLGuiView::FMLGuiView()
    : MainFrame(nullptr)
{
}

FMLGuiView::~FMLGuiView()
{
    MainFrame = nullptr;
}

void FMLGuiView::OnTransactionAdded(const FMLTransactionData& transactionData)
{
}

void FMLGuiView::OnTransactionRemoved(const int transactionId)
{
}

void FMLGuiView::OnTransactionUpdated(const FMLTransactionData& transactionData)
{
}

void FMLGuiView::OnTransactionsCleared()
{
}

void FMLGuiView::OnDataLoaded()
{
}

void FMLGuiView::OnDataSaved()
{
}
