/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslcertificateextensionproto.h"

#if QT_VERSION < 0x050000
void setupQSslCertificateExtensionProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQSslCertificateExtensionProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSslCertificateExtensionProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslCertificateExtension*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslCertificateExtension, proto);
  engine->globalObject().setProperty("QSslCertificateExtension",  constructor);
}

QSslCertificateExtensionProto::QSslCertificateExtensionProto(QObject *parent)
    : QObject(parent)
{
}

QSslCertificateExtensionProto::~QSslCertificateExtensionProto()
{
}

QScriptValue constructQSslCertificateExtension(QScriptContext * /*context*/, QScriptEngine  *engine)
{
  QSslCertificateExtension *obj = 0;
  obj = new QSslCertificateExtension();
  return engine->toScriptValue(obj);
}

bool QSslCertificateExtensionProto::isCritical() const
{
  QSslCertificateExtension *item = qscriptvalue_cast<QSslCertificateExtension*>(thisObject());
  if (item)
    return item->isCritical();
  return false;
}

bool QSslCertificateExtensionProto::isSupported() const
{
  QSslCertificateExtension *item = qscriptvalue_cast<QSslCertificateExtension*>(thisObject());
  if (item)
    return item->isSupported();
  return false;
}

QString QSslCertificateExtensionProto::name() const
{
  QSslCertificateExtension *item = qscriptvalue_cast<QSslCertificateExtension*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

QString QSslCertificateExtensionProto::oid() const
{
  QSslCertificateExtension *item = qscriptvalue_cast<QSslCertificateExtension*>(thisObject());
  if (item)
    return item->oid();
  return QString();
}

void QSslCertificateExtensionProto::swap(QSslCertificateExtension & other)
{
  QSslCertificateExtension *item = qscriptvalue_cast<QSslCertificateExtension*>(thisObject());
  if (item)
    item->swap(other);
}

QVariant QSslCertificateExtensionProto::value() const
{
  QSslCertificateExtension *item = qscriptvalue_cast<QSslCertificateExtension*>(thisObject());
  if (item)
    return item->value();
  return QVariant();
}

#endif
