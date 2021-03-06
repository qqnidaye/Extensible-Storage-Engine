// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "stdafx.h"

#pragma once

namespace Internal
{
    namespace Ese
    {
        namespace BlockCache
        {
            namespace Interop
            {
                template< class TM, class TN, class TW >
                public ref class JournalBase : public Base<TM, TN, TW>, IJournal
                {
                    public:

                        JournalBase( TM^ j ) : Base( j ) { }

                        JournalBase( TN** const ppj ) : Base( ppj ) {}

                    public:

                        virtual void GetProperties(
                            [Out] JournalPosition% journalPositionReplay,
                            [Out] JournalPosition% journalPositionDurableForWriteBack,
                            [Out] JournalPosition% journalPositionDurable );

                        virtual void VisitEntries( IJournal::VisitEntry^ visitEntry );

                        virtual void Repair( JournalPosition journalPositionInvalidate );

                        virtual JournalPosition AppendEntry( array<ArraySegment<byte>>^ payload );

                        virtual void Flush();

                        virtual void Truncate( JournalPosition journalPositionReplay );
                };

                template< class TM, class TN, class TW >
                inline void JournalBase<TM, TN, TW>::GetProperties(
                    [Out] JournalPosition% journalPositionReplay,
                    [Out] JournalPosition% journalPositionDurableForWriteBack,
                    [Out] JournalPosition% journalPositionDurable )
                {
                    ERR                 err                     = JET_errSuccess;
                    ::JournalPosition   jposReplay              = ::jposInvalid;
                    ::JournalPosition   jposDurableForWriteBack = ::jposInvalid;
                    ::JournalPosition   jposDurable             = ::jposInvalid;

                    journalPositionReplay = JournalPosition::Invalid;
                    journalPositionDurableForWriteBack = JournalPosition::Invalid;
                    journalPositionDurable = JournalPosition::Invalid;

                    Call( Pi->ErrGetProperties( &jposReplay, &jposDurableForWriteBack, &jposDurable ) );

                    journalPositionReplay = (JournalPosition)jposReplay;
                    journalPositionDurableForWriteBack = (JournalPosition)jposDurableForWriteBack;
                    journalPositionDurable = (JournalPosition)jposDurable;

                    return;

                HandleError:
                    journalPositionReplay = JournalPosition::Invalid;
                    journalPositionDurableForWriteBack = JournalPosition::Invalid;
                    journalPositionDurable = JournalPosition::Invalid;
                    throw EseException( err );
                }

                template<class TM, class TN, class TW>
                inline void JournalBase<TM, TN, TW>::VisitEntries( IJournal::VisitEntry^ visitEntry )
                {
                    ERR             err         = JET_errSuccess;
                    CVisitEntry*    pvisitentry = NULL;

                    Alloc( pvisitentry = new CVisitEntry( visitEntry ) );

                    Call( Pi->ErrVisitEntries(  ::IJournal::PfnVisitEntry( CVisitEntry::VisitEntry_ ),
                                                DWORD_PTR( pvisitentry ) ) );

                HandleError:
                    delete pvisitentry;
                    if ( err < JET_errSuccess )
                    {
                        throw EseException( err );
                    }
                }

                template<class TM, class TN, class TW>
                inline void JournalBase<TM, TN, TW>::Repair( JournalPosition journalPositionInvalidate )
                {
                    ERR err = JET_errSuccess;

                    Call( Pi->ErrRepair( (::JournalPosition)journalPositionInvalidate ) );

                    return;

                HandleError:
                    throw EseException( err );
                }

                template<class TM, class TN, class TW>
                inline JournalPosition JournalBase<TM, TN, TW>::AppendEntry( array<ArraySegment<byte>>^ payload )
                {
                    ERR                 err         = JET_errSuccess;
                    const size_t        cjb         = payload == nullptr ? 0 : payload->Length;
                    CJournalBuffer*     rgjb        = NULL;
                    ::JournalPosition   jpos        = ::jposInvalid;

                    if ( payload != nullptr )
                    {
                        Alloc( rgjb = new CJournalBuffer[ cjb + 1 ] );
                    }

                    for ( int ijb = 0; ijb < cjb; ijb++ )
                    {
                        const size_t    cb  = payload[ ijb ].Count;
                        BYTE*           rgb = NULL;

                        if ( payload[ ijb ].Array != nullptr )
                        {
                            Alloc( rgb = new BYTE[ cb + 1 ] );
                        }

                        rgjb[ ijb ] = CJournalBuffer( cb, rgb );

                        if ( cb )
                        {
                            pin_ptr<byte> rgbIn = &payload[ ijb ].Array[ payload[ ijb ].Offset ];
                            UtilMemCpy( rgb, (BYTE*)rgbIn, cb );
                        }
                    }

                    Call( Pi->ErrAppendEntry( cjb, rgjb, &jpos ) );
                    
                HandleError:
                    if ( rgjb != NULL )
                    {
                        for ( int ijb = 0; ijb < cjb; ijb++ )
                        {
                            delete[] rgjb[ ijb ].Rgb();
                        }
                    }
                    delete[] rgjb;
                    if ( err < JET_errSuccess )
                    {
                        throw EseException( err );
                    }
                    return (JournalPosition)jpos;
                }

                template<class TM, class TN, class TW>
                inline void JournalBase<TM, TN, TW>::Flush()
                {
                    ERR err = JET_errSuccess;

                    Call( Pi->ErrFlush() );

                    return;

                HandleError:
                    throw EseException( err );
                }

                template<class TM, class TN, class TW>
                inline void JournalBase<TM, TN, TW>::Truncate( JournalPosition journalPositionReplay )
                {
                    ERR err = JET_errSuccess;

                    Call( Pi->ErrTruncate( (::JournalPosition)journalPositionReplay ) );

                    return;

                HandleError:
                    throw EseException( err );
                }
            }
        }
    }
}
