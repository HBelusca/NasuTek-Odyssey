/*
 * Odyssey Explorer
 *
 * Copyright 2009 Andrew Hill <ash77 at domain odyssey.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
Used by the address band to dispatch navigation changes to the main browser object.

TODO:

*/
#include "precomp.h"

CBandProxy::CBandProxy()
{
}

CBandProxy::~CBandProxy()
{
}

HRESULT CBandProxy::FindBrowserWindow(IUnknown **browser)
{
	CComPtr<IServiceProvider>				serviceProvider;
	CComPtr<IWebBrowser2>					webBrowser;
	HRESULT									hResult;

	if (browser == NULL)
		return E_POINTER;
	hResult = fSite->QueryInterface(IID_IServiceProvider, (void **)&serviceProvider);
	if (FAILED (hResult))
		return hResult;
	hResult = serviceProvider->QueryService(SID_IWebBrowserApp, IID_IWebBrowser2, (void **)&webBrowser);
	if (FAILED (hResult))
		return hResult;
	*browser = webBrowser.Detach();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CBandProxy::SetSite(IUnknown *paramC)
{
	fSite = paramC;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CBandProxy::CreateNewWindow(long paramC)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBandProxy::GetBrowserWindow(IUnknown **paramC)
{
	if (paramC == NULL)
		return E_POINTER;
	return FindBrowserWindow(paramC);
}

HRESULT STDMETHODCALLTYPE CBandProxy::IsConnected()
{
	CComPtr<IUnknown>						webBrowser;
	HRESULT									hResult;

	hResult = FindBrowserWindow(&webBrowser);
	if (FAILED (hResult) || webBrowser.p == NULL)
		return S_FALSE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CBandProxy::NavigateToPIDL(LPCITEMIDLIST pidl)
{
	CComPtr<IOleWindow>						oleWindow;
	CComPtr<IServiceProvider>				serviceProvider;
	CComPtr<IUnknown>						webBrowserUnknown;
	CComPtr<IWebBrowser2>					webBrowser;
	HWND									browserWindow;
	CComVariant								args;
	CComVariant								emptyVariant;
	unsigned int							arraySize;
	HRESULT									hResult;

	hResult = FindBrowserWindow(&webBrowserUnknown);
	if (FAILED (hResult))
		return hResult;
	hResult = webBrowserUnknown->QueryInterface(IID_IWebBrowserApp, (void **)&webBrowser);
	if (FAILED (hResult))
		return hResult;
	hResult = webBrowser->put_Visible(TRUE);
	hResult = webBrowser->QueryInterface(IID_IServiceProvider, (void **)&serviceProvider);
	if (SUCCEEDED (hResult))
	{
		hResult = serviceProvider->QueryService(SID_STopLevelBrowser, IID_IOleWindow, (void **)&oleWindow);
		if (SUCCEEDED (hResult))
		{
			hResult = oleWindow->GetWindow(&browserWindow);
			if (IsIconic(browserWindow))
				ShowWindow(browserWindow, SW_RESTORE);
		}
	}
	arraySize = ILGetSize(pidl);
	V_VT(&args) = VT_ARRAY | VT_UI1;
	V_ARRAY(&args) = SafeArrayCreateVector(VT_UI1, 0, arraySize);
	if (V_ARRAY(&args) == NULL)
		return E_OUTOFMEMORY;
	memcpy(V_ARRAY(&args)->pvData, pidl, arraySize);
	hResult = webBrowser->Navigate2(&args, &emptyVariant, &emptyVariant, &emptyVariant, &emptyVariant);
	if (FAILED (hResult))
		return hResult;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CBandProxy::NavigateToURL(long paramC, long param10)
{
	return E_NOTIMPL;
}

HRESULT CreateBandProxy(REFIID riid, void **ppv)
{
	CComObject<CBandProxy>					*theBandProxy;
	HRESULT									hResult;

	if (ppv == NULL)
		return E_POINTER;
	*ppv = NULL;
	ATLTRY (theBandProxy = new CComObject<CBandProxy>);
	if (theBandProxy == NULL)
		return E_OUTOFMEMORY;
	hResult = theBandProxy->QueryInterface (riid, (void **)ppv);
	if (FAILED (hResult))
	{
		delete theBandProxy;
		return hResult;
	}
	return S_OK;
}
