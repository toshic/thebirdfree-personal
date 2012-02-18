/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
    Interface for state management functionality
	
FILE
	state.h
*/


#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H


typedef enum
{
	/* Application has not been initialised */
	PbapStateUninitialised,
	/* Connection Library has been initialised */
	PbapStateConInited,
	/* Application has completed initialisation and is idle */
	PbapStateAppIdle,
	/* Application has started a connection */
	PbapStateConnecting,
	/* Application has started a connection */
	PbapStateConnected,
	/* Remote Client is downloading the vCard List */
	PbapStatePullvCardList,
	/* Remote Client is downloading the vCard Entry */
	PbapStatePullvCardEntry,
	/* Remote Client is downloading a complete phonebook */
	PbapStatePullPhonebook,
	/* Remote Client is requesting the size of a phonebook */
	PbapStatePullPhonebookSize,
	
	PbapStateEndOfList
} pbap_states;


void setState(pbap_states *pState, pbap_states pNewState);

#endif /* APPLICATION_STATE_H */
