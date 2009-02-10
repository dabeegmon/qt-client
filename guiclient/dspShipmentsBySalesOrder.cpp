/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspShipmentsBySalesOrder.h"

#include <QMenu>
#include <Q3Process>
#include <QMessageBox>
#include <QRegExp>
#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include <openreports.h>

#include "inputManager.h"
#include "salesOrderList.h"
#include "printShippingForm.h"

dspShipmentsBySalesOrder::dspShipmentsBySalesOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_salesOrder, SIGNAL(newId(int)), this, SLOT(sFillList(int)));
  connect(_salesOrderList, SIGNAL(clicked()), this, SLOT(sSalesOrderList()));
  connect(_soship, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_salesOrder, SIGNAL(requestList()), this, SLOT(sSalesOrderList()));

#ifndef Q_WS_MAC
  _salesOrderList->setMaximumWidth(25);
#endif

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _salesOrder, SLOT(setId(int)));

  _soship->setRootIsDecorated(TRUE);
  _soship->addColumn(tr("Shipment #"),         _orderColumn, Qt::AlignLeft,   true,  "shiphead_number"   );
  _soship->addColumn(tr("Ship Date"),           _itemColumn, Qt::AlignCenter, true,  "shiphead_shipdate" );
  _soship->addColumn(tr("#"),                   _seqColumn,  Qt::AlignCenter, true,  "linenumber" );
  _soship->addColumn(tr("Item"),                _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _soship->addColumn(tr("Description"),         -1,          Qt::AlignLeft,   true,  "itemdescription"   );
  _soship->addColumn(tr("Site"),                _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _soship->addColumn(tr("Ordered"),             _qtyColumn,  Qt::AlignRight,  true,  "qtyord"  );
  _soship->addColumn(tr("Shipped"),             _qtyColumn,  Qt::AlignRight,  true,  "qtyshipped"  );
  _soship->addColumn(tr("Tracking Number"),     _qtyColumn,  Qt::AlignRight,  true,  "shiphead_tracknum"  );
  _soship->addColumn(tr("Freight at Shipping"), _qtyColumn,  Qt::AlignRight,  true,  "shiphead_freight"  );
}

dspShipmentsBySalesOrder::~dspShipmentsBySalesOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspShipmentsBySalesOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspShipmentsBySalesOrder::set(const ParameterList &pParams)
{ 
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _salesOrder->setId(param.toInt());
    _salesOrder->setEnabled(FALSE);
  }

  return NoError;
}

void dspShipmentsBySalesOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Print Shipping Form..."), this, SLOT(sPrintShippingForm()), 0);
  if (!_privileges->check("PrintBillsOfLading"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Query Shipment Status..."), this, SLOT(sFillURL()), 0);
}

void dspShipmentsBySalesOrder::sPrint()
{
  ParameterList params;
  params.append("sohead_id", _salesOrder->id());

  orReport report("ShipmentsBySalesOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspShipmentsBySalesOrder::sPrintShippingForm()
{
  ParameterList params;
  params.append("cosmisc_id", _soship->id());

  printShippingForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspShipmentsBySalesOrder::sSalesOrderList()
{
  ParameterList params;
  params.append("sohead_id", _salesOrder->id());
  params.append("soType", (cSoOpen | cSoClosed));
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  if(id != QDialog::Rejected)
    _salesOrder->setId(id);
}

void dspShipmentsBySalesOrder::sFillList(int pSoheadid)
{
  _soship->clear();

  if (pSoheadid != -1)
  {
    q.prepare( "SELECT cohead_number,"
               "       formatDate(cohead_orderdate) AS orderdate,"
               "       cohead_custponumber,"
               "       cust_name, cust_phone "
               "FROM cohead, cust "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (cohead_id=:sohead_id) );" );
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.first())
    {
      _orderDate->setText(q.value("orderdate").toString());
      _poNumber->setText(q.value("cohead_custponumber").toString());
      _custName->setText(q.value("cust_name").toString());
      _custPhone->setText(q.value("cust_phone").toString());
    }

    ParameterList params;
    params.append("sohead_id", _salesOrder->id());
    MetaSQLQuery fillm = mqlLoad("shipments", "detail");
    q = fillm.toQuery(params);
    if (q.first())
    {
      _soship->populate(q, true);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
  }
}

void dspShipmentsBySalesOrder::sFillURL()
{
    QString url;
    
     q.prepare( "SELECT upper(cosmisc_shipvia) AS cosmisc_shipvia, cosmisc_tracknum, cohead_shiptozipcode "
               "FROM cosmisc, cohead "
               "WHERE ((cosmisc_id=:cosmisc_id) "
               "AND (cosmisc_cohead_id=cohead_id));");
     
    q.bindValue(":cosmisc_id", _soship->id());
    q.exec();
    if (q.first()) {
     bool findShipper;
     findShipper = false;
     
// Code for UPS	
     if (q.value("cosmisc_shipvia").toString ().left(3) == "UPS") {
       QString url("http://wwwapps.ups.com/WebTracking/processInputRequest?HTMLVersion=5.0&loc=en_US&Requester=UPSHome&tracknum=");
       url +=  q.value("cosmisc_tracknum").toString ();
       url +=  "&AgreeToTermsAndConditions=yes&track.x=40&track.y=9";
       findShipper = true;
        omfgThis->launchBrowser(this, url);
      }
     
 // Code for SAIA	
     if (q.value("cosmisc_shipvia").toString ().left(4) == "SAIA") {
       QString url("http://www.SaiaSecure.com/tracing/manifest.asp?UID=&PWD=&PRONum1=");
       QString _partial;
       QString _tracknum;
       _tracknum = q.value("cosmisc_tracknum").toString ();
       int _length_tracknum;
       int _how_many;
       _length_tracknum = _tracknum.length();
       _how_many = _length_tracknum - 5;
       _partial = _tracknum.left(3);
       _partial += _tracknum.mid( 4, _how_many);
       url +=  _partial;
       url +=  "&Type=Pro&x=55&y=8";
       findShipper = true;
        omfgThis->launchBrowser(this, url);
      }

 // Code for A&B
     if ((q.value("cosmisc_shipvia").toString ().left(5) == "A & B") || (q.value("cosmisc_shipvia").toString ().left(3) == "A&B")) {
       QString url("http://www.aandbfreight.com/");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("A & B"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for AVERITT
     if (q.value("cosmisc_shipvia").toString ().left(7) == "AVERITT") {
       QString url("http://www.averittexpress.com/");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("AVERITT"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for DHL	
     if (q.value("cosmisc_shipvia").toString ().left(3) == "DHL") {
       QString url("http://www.dhl-usa.com/home/Home.asp");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("DHL"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for FEDEX	
     if (q.value("cosmisc_shipvia").toString ().left(5) == "FEDEX") {
       QString url("http://www.fedex.com/Tracking?ascend_header=1&clienttype=dotcom&cntry_code=us&language=english&tracknumbers=");
       QString _partial;
       QString _tracknum;
       _tracknum = q.value("cosmisc_tracknum").toString ();
       int _length_tracknum;
       int _how_many;
       _length_tracknum = _tracknum.length();
       _how_many = _length_tracknum - 5;
       QString _check_tail;
       _check_tail = _tracknum.right(2);
       _check_tail = _check_tail.left(1);
       if (_check_tail == "-") {
       // Ok we need to strip the hyphen out
         _partial = _tracknum.left( _length_tracknum -2);
         _partial += _tracknum.right(1);
       }
       else
       {
         _partial = _tracknum;
       }
       url +=  _partial;
       findShipper = true;
        omfgThis->launchBrowser(this, url);
      }

 // Code for R&L	
     if ((q.value("cosmisc_shipvia").toString ().left(5) == "R & L") || (q.value("cosmisc_shipvia").toString ().left(3) == "R&L"))  {       QString url("http://www.rlcarriers.com/new/index.asp");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("R&L"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for ROADWAY	
     if (q.value("cosmisc_shipvia").toString ().left(7) == "ROADWAY") {
       QString url("http://www.quiktrak.roadway.com/cgi-bin/quiktrak?type=0&pro0=");
       url += q.value("cosmisc_tracknum").toString ();
       url += "&zip0=";
       url += q.value("cohead_shiptozipcode").toString ();
       findShipper = true;
       omfgThis->launchBrowser(this, url);
      }

 // Code for USF	
     if (q.value("cosmisc_shipvia").toString ().left(3) == "USF") {
       QString url("http://www.usfc.com/shipmentStatus/shipmentStatustWS.jsp?TrackType=H&TrackNumber=");
       url +=  q.value("cosmisc_tracknum").toString ();
       findShipper = true;
       omfgThis->launchBrowser(this, url);
      }

 // Code for WATKINS	
     if (q.value("cosmisc_shipvia").toString ().left(7) == "WATKINS") {
       QString url("http://www.watkins.com/");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("WATKINS"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for YELLOW	
     if (q.value("cosmisc_shipvia").toString ().left(7) == "YELLOW") {
       QString url("http://www.myyellow.com/dynamic/services/content/index.jsp");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("YELLOW"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

     if (!findShipper){
      QMessageBox::information(this, tr("Shipper"), tr("We do not currently process this shipper ") + q.value("cosmisc_shipvia").toString (), QMessageBox::Ok);
     }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
}
