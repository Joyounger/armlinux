/*
 * $Id: tcpslavebase.cpp,v 1.1.2.3 2001/10/11 21:29:43 hausmann Exp $
 *
 * Copyright (C) 2000 Alex Zepeda <jazepeda@pacbell.net>
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>
 *
 * This file is part of the KDE project
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "tcpslavebase.h"
#include "resolver.h"

#include <qsocketdevice.h>

#include <assert.h>

using namespace KIO;

class TCPSlaveBase::TcpSlaveBasePrivate
{
public:
    int status;
    int timeout;
    bool block;
    
    QSocketDevice socket;
};


TCPSlaveBase::TCPSlaveBase(unsigned short int default_port,
                           const QCString &protocol,
                           const QCString &pool_socket,
                           const QCString &app_socket)
             :SlaveBase (protocol, pool_socket, app_socket),
              m_iSock(-1), m_iDefaultPort(default_port),
	     m_sServiceName(protocol) // , fp(0)
{
    // We have to have two constructors, so don't add anything
    // else in here. Put it in doConstructorStuff() instead.
        doConstructorStuff();
        m_bIsSSL = false;
}

TCPSlaveBase::TCPSlaveBase(unsigned short int default_port,
                           const QCString &protocol,
                           const QCString &pool_socket,
                           const QCString &app_socket,
                           bool useSSL)
             :SlaveBase (protocol, pool_socket, app_socket),
              m_iSock(-1), m_bIsSSL(useSSL), m_iDefaultPort(default_port),
	     m_sServiceName(protocol) // , fp(0)
{
        doConstructorStuff();
        if (useSSL)
           m_bIsSSL = InitializeSSL();
}

// The constructor procedures go here now.
void TCPSlaveBase::doConstructorStuff()
{
    d = new TcpSlaveBasePrivate;
    d->status = -1;
    d->timeout = -1;
    d->block = false;
    // ### quick HACK
    if ( m_iDefaultPort == 0 && m_sServiceName == "http" )
	m_iDefaultPort = 80;
}

TCPSlaveBase::~TCPSlaveBase()
{
    CleanSSL();
    delete d;
}

int TCPSlaveBase::Write(const void *data, int len)
{
    return d->socket.writeBlock( (const char *)data, len );
}

int TCPSlaveBase::Read(void *data, int len)
{
    return d->socket.readBlock( (char *)data, len );
}

int TCPSlaveBase::ReadLine(char *data, int len)
{
    // let's not segfault!
    if (!data) return -1;

    *data = 0;
    // ugliness alert!!  calling read() so many times can't be good...
    int clen = 0;
    char *buf = data;
    while (clen < len) {
        int rc = d->socket.readBlock(buf, 1);
	if (rc < 0) return -1;
	clen++;
	if (*buf++ == '\n')
	    break;
    }
    *buf = 0;
    return clen;
}

unsigned short int TCPSlaveBase::GetPort(unsigned short int _port)
{
    unsigned short int port = _port;
    if (_port <= 0) {
	/*
        struct servent *srv=getservbyname(m_sServiceName, "tcp");
        if (srv) {
            port=ntohs(srv->s_port);
        }
        else */
            port=m_iDefaultPort;
    }
    return port;
}

  // This function is simply a wrapper to establish the connection
  // to the server.  It's a bit more complicated than ::connect
  // because we first have to check to see if the user specified
  // a port, and if so use it, otherwise we check to see if there
  // is a port specified in /etc/services, and if so use that
  // otherwise as a last resort use the supplied default port.
bool TCPSlaveBase::ConnectToHost(const QCString &host,
                                 unsigned short int _port)
{
    return ConnectToHost( host, _port, true );
}

bool TCPSlaveBase::ConnectToHost( const QString &host,
                                  unsigned int _port,
                                  bool sendError )
{
    QHostAddress address;

    m_iSock = 0;

    if ( !Resolver::self()->resolve( host, address ) )
    {
	if ( sendError )
	    error( ERR_UNKNOWN_HOST, host);
	d->status = IO_LookupError;
	return false;
    }

    d->status = -1;

    if ( !d->socket.connect( address, _port != 0 ? _port : m_iDefaultPort ) )
    {
	if ( sendError )
	    error( ERR_COULD_NOT_CONNECT, host);
	return false;
    }

    m_iSock = d->socket.socket();

    return true;
    
    /*
    unsigned short int port;
    KExtendedSocket ks;

   //  - leaving SSL - warn before we even connect
   if (metaData("ssl_activate_warnings") == "TRUE" &&
              metaData("ssl_was_in_use") == "TRUE" &&
       !m_bIsSSL) {
      KSSLSettings kss;
      if (kss.warnOnLeave()) {
      int result = messageBox( WarningContinueCancel,
                                     i18n("You are about to leave secure "
                                          "mode.  Transmissions will no "
                                          "longer be encrypted.\nThis "
                                          "means that a third party could "
                                          "observe your data in transit."),
                                     i18n("Security information"),
                                     i18n("Continue Loading") );
      if ( result == KMessageBox::Cancel )
         return -1;
      }
   }

    d->status = -1;
    d->host = host;
    d->needSSLHandShake = m_bIsSSL;
    port = GetPort(_port);
    ks.setAddress(host, port);
    if ( d->timeout > -1 )
        ks.setTimeout( d->timeout );

    if (ks.connect() < 0)
    {
        d->status = ks.status();
        if ( sendError )
        {
            if (d->status == IO_LookupError)
                error( ERR_UNKNOWN_HOST, host);
            else if ( d->status != -1 )
                error( ERR_COULD_NOT_CONNECT, host);
        }
        return false;
    }

    m_iSock = ks.fd();

    // store the IP for later
    const KSocketAddress *sa = ks.peerAddress();
    d->ip = sa->nodeName();

    ks.release();		// KExtendedSocket no longer applicable

    if ( d->block != ks.blockingMode() )
        ks.setBlockingMode( d->block );

    m_iPort=port;

    if (m_bIsSSL && !d->useSSLTunneling) {
        if ( !doSSLHandShake( sendError ) )
            return false;
    }
    else
        setMetaData("ssl_in_use", "FALSE");

    // Since we want to use stdio on the socket,
    // we must fdopen it to get a file pointer,
    // if it fails, close everything up
    if ((fp = fdopen(m_iSock, "w+")) == 0) {
        CloseDescriptor();
        return false;
    }

    return true;
    */
}

void TCPSlaveBase::CloseDescriptor()
{
    /*
    stopTLS();
    if (fp) {
        fclose(fp);
        fp=0;
        m_iSock=-1;
        if (m_bIsSSL)
            d->kssl->close();
    }
    if (m_iSock != -1) {
        close(m_iSock);
        m_iSock=-1;
    }
    */
    d->socket.close();
    m_iSock = -1;
}

bool TCPSlaveBase::InitializeSSL()
{
    /*
    if (m_bIsSSL) {
        if (KSSL::doesSSLWork()) {
            d->kssl = new KSSL;
            return true;
        }
        else
            return false;
    }
    else
        return false;
    */
    return false;
}

void TCPSlaveBase::CleanSSL()
{
    /*
    delete d->cc;

    if (m_bIsSSL) {
        delete d->kssl;
    }
    */
}

bool TCPSlaveBase::AtEOF()
{
    return d->socket.atEnd();
    //    return feof(fp);
}

bool TCPSlaveBase::startTLS()
{
    /*
    if (d->usingTLS || d->useSSLTunneling || m_bIsSSL || !KSSL::doesSSLWork())
        return false;

    d->kssl = new KSSL(false);
    if (!d->kssl->TLSInit()) {
        delete d->kssl;
        return -1;
    }

    certificatePrompt();

    int rc = d->kssl->connect(m_iSock);
    if (rc < 0) {
        delete d->kssl;
        return -2;
    }

    d->usingTLS = true;
    setMetaData("ssl_in_use", "TRUE");
    rc = verifyCertificate();
    if (rc != 1) {
        setMetaData("ssl_in_use", "FALSE");
        d->usingTLS = false;
        delete d->kssl;
        return -3;
    }

    return (d->usingTLS ? 1 : 0);
    */
    return false;
}


void TCPSlaveBase::stopTLS()
{
    /*
    if (d->usingTLS) {
        delete d->kssl;
        d->usingTLS = false;
        setMetaData("ssl_in_use", "FALSE");
    }
    */
}


bool TCPSlaveBase::canUseTLS()
{
    /*
    if (m_bIsSSL || d->needSSLHandShake || !KSSL::doesSSLWork())
        return false;

    KSSLSettings kss;
    return kss.tlsv1();
    */
    return false;
}


void TCPSlaveBase::certificatePrompt()
{
    /*
QString certname;   // the cert to use this session
bool send = false, prompt = false, save = false, forcePrompt = false;
KSSLCertificateHome::KSSLAuthAction aa;

  setMetaData("ssl_using_client_cert", "FALSE"); // we change this if needed

  if (metaData("ssl_no_client_cert") == "TRUE") return;
  forcePrompt = (metaData("ssl_force_cert_prompt") == "TRUE");

  // Delete the old cert since we're certainly done with it now
  if (d->pkcs) {
     delete d->pkcs;
     d->pkcs = NULL;
  }

  if (!d->kssl) return;

  // Look for a general certificate
  if (!forcePrompt) {
        certname = KSSLCertificateHome::getDefaultCertificateName(&aa);
        switch(aa) {
        case KSSLCertificateHome::AuthSend:
          send = true; prompt = false;
         break;
        case KSSLCertificateHome::AuthDont:
          send = false; prompt = false;
          certname = "";
         break;
        case KSSLCertificateHome::AuthPrompt:
          send = false; prompt = true;
         break;
        default:
         break;
        }
  }


  // Look for a certificate on a per-host basis as an override
  QString tmpcn = KSSLCertificateHome::getDefaultCertificateName(d->host, &aa);
  if (aa != KSSLCertificateHome::AuthNone) {   // we must override
    switch (aa) {
        case KSSLCertificateHome::AuthSend:
          send = true; prompt = false;
          certname = tmpcn;
         break;
        case KSSLCertificateHome::AuthDont:
          send = false; prompt = false;
          certname = "";
         break;
        case KSSLCertificateHome::AuthPrompt:
          send = false; prompt = true;
          certname = tmpcn;
         break;
        default:
         break;
    }
  }

  // Finally, we allow the application to override anything.
  if (hasMetaData("ssl_demand_certificate")) {
     certname = metaData("ssl_demand_certificate");
     if (!certname.isEmpty()) {
        forcePrompt = false;
        prompt = false;
        send = true;
     }
  }

  if (certname.isEmpty() && !prompt && !forcePrompt) return;

  // Ok, we're supposed to prompt the user....
  if (prompt || forcePrompt) {
     QStringList certs = KSSLCertificateHome::getCertificateList();

        if (certs.isEmpty()) return;  // we had nothing else, and prompt failed

     if (!d->dcc) {
        d->dcc = new DCOPClient;
        d->dcc->attach();
        if (!d->dcc->isApplicationRegistered("kio_uiserver")) {
           KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
                                                   QStringList() );
        }
     }

     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << d->host+":"+QString::number(m_iPort);
     arg << certs;
     bool rc = d->dcc->call("kio_uiserver", "UIServer",
                               "showSSLCertDialog(QString, QStringList)",
                               data, rettype, retval);

     if (rc && rettype == "KSSLCertDlgRet") {
        QDataStream retStream(retval, IO_ReadOnly);
        KSSLCertDlgRet drc;
        retStream >> drc;
        if (drc.ok) {
           send = drc.send;
           save = drc.save;
           certname = drc.choice;
        }
     }
  }

    // The user may have said to not send the certificate,
    // but to save the choice
  if (!send) {
     if (save) {
            KSSLCertificateHome::setDefaultCertificate(certname, d->host,
                                                       false, false);
     }
     return;
  }

  // We're almost committed.  If we can read the cert, we'll send it now.
  KSSLPKCS12 *pkcs = KSSLCertificateHome::getCertificateByName(certname);
  if (!pkcs) {           // We need the password
     do {
        QString pass;
        QByteArray authdata, authval;
        QCString rettype;
        KIO::AuthInfo ai;
        QDataStream qds(authdata, IO_WriteOnly);
        ai.prompt = i18n("Enter the certificate password:");
        ai.caption = i18n("SSL Certificate Password");
        ai.setModified(true);
        ai.username = certname;
        ai.keepPassword = false;
        qds << ai;

        if (!d->dcc) {
           d->dcc = new DCOPClient;
           d->dcc->attach();
           if (!d->dcc->isApplicationRegistered("kio_uiserver")) {
              KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
                                                      QStringList() );
           }
        }

        bool rc = d->dcc->call("kio_uiserver", "UIServer",
                                   "openPassDlg(KIO::AuthInfo)",
                                   authdata, rettype, authval);
        if (!rc) break;
        if (rettype != "QByteArray") continue;

        QDataStream qdret(authval, IO_ReadOnly);
        QByteArray authdecode;
        qdret >> authdecode;
        QDataStream qdtoo(authdecode, IO_ReadOnly);
        qdtoo >> ai;
        if (!ai.isModified()) break;
        pass = ai.password;

        pkcs = KSSLCertificateHome::getCertificateByName(certname, pass);

        if (!pkcs) {
              int rc = messageBox(WarningYesNo, i18n("Couldn't open the "
                                                     "certificate.  Try a "
                                                     "new password?"),
                                                i18n("SSL"));
              if (rc == KMessageBox::No) break;
        }
     } while (!pkcs);
  }

   // If we could open the certificate, let's send it
   if (pkcs) {
      if (!d->kssl->setClientCertificate(pkcs)) {
            messageBox(Information, i18n("Sorry, the procedure to set the "
                                         "client certificate for the session "
                                         "failed."), i18n("SSL"));
         delete pkcs;  // we don't need this anymore
      } else {
         kdDebug(7029) << "Client SSL certificate is being used." << endl;
         setMetaData("ssl_using_client_cert", "TRUE");
         if (save) {
                KSSLCertificateHome::setDefaultCertificate(certname, d->host,
                                                           true, false);
         }
      }
      d->pkcs = pkcs;
   }
    */
}




bool TCPSlaveBase::usingTLS()
{
    //    return d->usingTLS;
    return false;
}


//  Returns 0 for failed verification, -1 for rejected cert and 1 for ok
int TCPSlaveBase::verifyCertificate()
{
    /*
    int rc = 0;
    bool permacache = false;
    bool isChild = false;
    QString theurl = QString(m_sServiceName)+"://"+d->host+":"+QString::number(m_iPort);
    bool _IPmatchesCN = false;

    if (!d->cc) d->cc = new KSSLCertificateCache;

    KSSLCertificate& pc = d->kssl->peerInfo().getPeerCertificate();

    KSSLCertificate::KSSLValidation ksv = pc.validate();
    */
    /* Setting the various bits of meta-info that will be needed. */
    /*
    setMetaData("ssl_peer_cert_subject", pc.getSubject());
    setMetaData("ssl_peer_cert_issuer", pc.getIssuer());
    setMetaData("ssl_cipher", d->kssl->connectionInfo().getCipher());
    setMetaData("ssl_cipher_desc",
                            d->kssl->connectionInfo().getCipherDescription());
    setMetaData("ssl_cipher_version",
                                d->kssl->connectionInfo().getCipherVersion());
    setMetaData("ssl_cipher_used_bits",
              QString::number(d->kssl->connectionInfo().getCipherUsedBits()));
    setMetaData("ssl_cipher_bits",
                  QString::number(d->kssl->connectionInfo().getCipherBits()));
    setMetaData("ssl_peer_ip", d->ip);
    setMetaData("ssl_cert_state", QString::number(ksv));
    setMetaData("ssl_good_from", pc.getNotBefore());
    setMetaData("ssl_good_until", pc.getNotAfter());

   if (ksv == KSSLCertificate::Ok) {
      rc = 1;
      setMetaData("ssl_action", "accept");
   }

   _IPmatchesCN = d->kssl->peerInfo().certMatchesAddress();

   kdDebug(7029) << "SSL HTTP frame the parent? " << metaData("main_frame_request") << endl;
   if (!hasMetaData("main_frame_request") || metaData("main_frame_request") == "TRUE") {
      // Since we're the parent, we need to teach the child.
      setMetaData("ssl_parent_ip", d->ip);
      setMetaData("ssl_parent_cert", pc.toString());
      //  - Read from cache and see if there is a policy for this
      KSSLCertificateCache::KSSLCertificatePolicy cp =
                                         d->cc->getPolicyByCertificate(pc);

      //  - validation code
      if (ksv != KSSLCertificate::Ok || !_IPmatchesCN) {
         if (cp == KSSLCertificateCache::Unknown || 
             cp == KSSLCertificateCache::Ambiguous) {
            cp = KSSLCertificateCache::Prompt;
         } else {
            // A policy was already set so let's honour that.
            permacache = d->cc->isPermanent(pc);
         }

         // Precondition: cp is one of Reject, Accept or Prompt

         switch (cp) {
         case KSSLCertificateCache::Accept:
           rc = 1;
           setMetaData("ssl_action", "accept");
          break;
         case KSSLCertificateCache::Reject:
           rc = -1;
           setMetaData("ssl_action", "reject");
          break;
         case KSSLCertificateCache::Prompt:
           {
           int result;
             do {
                if (ksv == KSSLCertificate::Ok && !_IPmatchesCN) {
                        QString msg = i18n("The IP address of the host %1 "
                                           "does not match the one the "
                                           "certificate was issued to.");
                   result = messageBox( WarningYesNoCancel,
                              msg.arg(d->host),
                              i18n("Server Authentication"),
                              i18n("&Details..."),
                              i18n("Co&ntinue") );
                } else {
                   QString msg = i18n("The server certificate failed the "
                                      "authenticity test (%1).");
                   result = messageBox( WarningYesNoCancel,
                              msg.arg(d->host),
                              i18n("Server Authentication"),
                              i18n("&Details..."),
                              i18n("Co&ntinue") );
                }

                if (result == KMessageBox::Yes) {
                   if (!d->dcc) {
                      d->dcc = new DCOPClient;
                      d->dcc->attach();
                   }
                   QByteArray data, ignore;
                   QCString ignoretype;
                   QDataStream arg(data, IO_WriteOnly);
                   arg << theurl << mOutgoingMetaData;
                        d->dcc->call("kio_uiserver", "UIServer",
                                "showSSLInfoDialog(QString,KIO::MetaData)",
                                data, ignoretype, ignore);
                }
             } while (result == KMessageBox::Yes);

             if (result == KMessageBox::No) {
                setMetaData("ssl_action", "accept");
                rc = 1;
                cp = KSSLCertificateCache::Accept;
                   result = messageBox( WarningYesNo,
                                  i18n("Would you like to accept this "
                                       "certificate forever without "
                                       "being prompted?"),
                                  i18n("Server Authentication"),
                                         i18n("&Forever"),
                                         i18n("&Current Sessions Only"));
                    if (result == KMessageBox::Yes)
                        permacache = true;
                    else
                        permacache = false;
             } else {
                setMetaData("ssl_action", "reject");
                rc = -1;
                cp = KSSLCertificateCache::Prompt;
             }
          break;
            }
         default:
          kdDebug(7029) << "TCPSlaveBase/SSL error in cert code."
                              << "Please report this to kfm-devel@kde.org."
                              << endl;
          break;
         }
      }


      //  - cache the results
      d->cc->addCertificate(pc, cp, permacache);
      d->cc->saveToDisk();    // So that other slaves can get at it.
      // FIXME: we should be able to notify other slaves of this here.
    } else {    // Child frame
      //  - Read from cache and see if there is a policy for this
      KSSLCertificateCache::KSSLCertificatePolicy cp =
                                             d->cc->getPolicyByCertificate(pc);
      isChild = true;

      // Check the cert and IP to make sure they're the same
      // as the parent frame
      bool certAndIPTheSame = (d->ip == metaData("ssl_parent_ip") &&
                               pc.toString() == metaData("ssl_parent_cert"));

      if (ksv == KSSLCertificate::Ok && _IPmatchesCN) {
        if (certAndIPTheSame) {       // success
          rc = 1;
        } else {
          if (true) {     // success   FIXME: prompt to continue
            rc = 1;
          } else {    // fail
            rc = -1;
          }
        }
      } else {
        if (cp == KSSLCertificateCache::Accept) {
           if (certAndIPTheSame) {    // success
             rc = 1;
           } else {   // fail
             rc = -1;
           }
        } else {      // fail
          rc = -1;
        }
      }
      // FIXME: failure message boxes are needed
    }


   if (rc == -1) return rc;

   // Things to check:
   //  - posting unencrypted data  -- elsewhere?
   //                 - transmitting any data unencrypted?  In the app??
   //                         singleton in write()?

   if (metaData("ssl_activate_warnings") == "TRUE") {
   //  - entering SSL
   if (!isChild && metaData("ssl_was_in_use") == "FALSE" &&
                                        d->kssl->settings()->warnOnEnter()) {
     int result;
     do {
                result = messageBox( WarningYesNo, i18n("You are about to "
                                                        "enter secure mode.  "
                                                        "All transmissions "
                                                        "will be encrypted "
                                                        "unless otherwise "
                                                        "noted.\nThis means "
                                                        "that no third party "
                                                        "will be able to "
                                                        "easily observe your "
                                                        " data in transfer."),
                                                   i18n("Security information"),
                                                   i18n("Display SSL "
                                                        "Information"),
                                                   i18n("Continue") );
      if ( result == KMessageBox::Yes )
      {
          if (!d->dcc) {
             d->dcc = new DCOPClient;
             d->dcc->attach();
          }
          QByteArray data, ignore;
          QCString ignoretype;
          QDataStream arg(data, IO_WriteOnly);
          arg << theurl << mOutgoingMetaData;
          d->dcc->call("kio_uiserver", "UIServer",
                       "showSSLInfoDialog(QString,KIO::MetaData)",
                       data, ignoretype, ignore);
      }
      } while (result != KMessageBox::No);
   }

   #if 0
   // This will probably go to khtml_part
   //  - mixed SSL/nonSSL
        // I assert that if any two portions of a loaded document are of
        // opposite SSL status then either one of them must be different
        // than the parent.  Therefore we can only compare each child
        // against the parent both here and in non-SSL mode.
        // The problem which remains is how to have this notification
        // appear only once per page.
   if ( isChild && d->kssl->settings()->warnOnMixed() &&
        metaData("ssl_was_in_use") != "TRUE") {
      // FIXME: do something!
   }
   #endif
   }   // if ssl_activate_warnings


   kdDebug(7029) << "SSL connection information follows:" << endl
          << "+-----------------------------------------------" << endl
          << "| Cipher: " << d->kssl->connectionInfo().getCipher() << endl
          << "| Description: " << d->kssl->connectionInfo().getCipherDescription() << endl
          << "| Version: " << d->kssl->connectionInfo().getCipherVersion() << endl
          << "| Strength: " << d->kssl->connectionInfo().getCipherUsedBits()
          << " of " << d->kssl->connectionInfo().getCipherBits()
          << " bits used." << endl
          << "| PEER:" << endl
          << "| Subject: " << d->kssl->peerInfo().getPeerCertificate().getSubject() << endl
          << "| Issuer: " << d->kssl->peerInfo().getPeerCertificate().getIssuer() << endl
          << "| Validation: " << (int)ksv << endl
          << "| Certificate matches IP: " << _IPmatchesCN << endl
          << "+-----------------------------------------------"
          << endl;

   // sendMetaData();  Do not call this function!!
   return rc;
*/
   return 0;
}


bool TCPSlaveBase::isConnectionValid()
{
    /*
    if ( m_iSock == -1 )
      return false;

    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(m_iSock , &rdfs);

    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 0;
    int retval = select(m_iSock+1, &rdfs, NULL, NULL, &tv);
    // retval ==  0 ==> Connection Idle
    // retval >=  1 ==> Connection Active
    if ( retval == -1 )
        return false;       // should really never happen, but just in-case...
    else if ( retval > 0 )
    {
      char buffer[100];
      retval = recv(m_iSock, buffer, 80, MSG_PEEK);
      // retval ==  0 ==> Connection closed
      if ( retval == 0 )
        return false;
    }
    return true;
    */
    // ### HACK
    return d->socket.waitForMore( 0 ) != -1 ? true : false;
}


bool TCPSlaveBase::waitForResponse( int t )
{
    /*
    fd_set rd, wr;
    struct timeval timeout;

    int n = t; // Timeout in seconds
    while(n--)
    {
        FD_ZERO(&rd);
        FD_ZERO(&wr);
        FD_SET(m_iSock, &rd);

        timeout.tv_usec = 0;
        timeout.tv_sec = 1; // 1 second

        select(m_iSock+1, &rd, &wr, (fd_set *)0, &timeout);

        if (FD_ISSET(m_iSock, &rd))
            return true;
    }
    return false; // Timed out!
    */
    return d->socket.waitForMore( t ) != -1 ? true : false;
}

int TCPSlaveBase::connectResult()
{
    //    return d->status;
    return d->status == -1 ? d->socket.status() : d->status;
}

void TCPSlaveBase::setBlockConnection( bool b )
{
    if ( !b )
	assert( false ); // ### not supported
    //    d->block = b;
}

void TCPSlaveBase::setConnectTimeout( int t )
{
    // ### grmblll, QSocketDevice does not support it
    d->timeout = t;
}

bool TCPSlaveBase::isSSLTunnelEnabled()
{
    //    return d->useSSLTunneling;
    return false;
}

void TCPSlaveBase::setEnableSSLTunnel( bool enable )
{
    if ( enable )
	assert( false );
    //    d->useSSLTunneling = enable;
}

/*
bool TCPSlaveBase::doSSLHandShake( bool sendError )
{
    d->kssl->reInitialize();
    certificatePrompt();
    d->status = d->kssl->connect(m_iSock);
    if (d->status < 0) {
        CloseDescriptor();
        if ( sendError )
            error( ERR_COULD_NOT_CONNECT, d->host);
        return false;
    }
    setMetaData("ssl_in_use", "TRUE");
    int rc = verifyCertificate();
    if ( rc != 1 ) {
        d->status = -1;
        CloseDescriptor();
        if ( sendError )
            error( ERR_COULD_NOT_CONNECT, d->host);
        return false;
    }
    d->needSSLHandShake = false;
    return true;
    return false;
}
*/
