@ stdcall TTCharToUnicode(ptr ptr long ptr long long)
@ stdcall TTDeleteEmbeddedFont(ptr long ptr)
@ stdcall TTEmbedFont(ptr long long ptr ptr ptr ptr ptr long long ptr)
@ stdcall TTEmbedFontFromFileA(ptr str long long long ptr ptr ptr ptr ptr long long ptr)
@ stdcall TTEnableEmbeddingForFacename(str long)
@ stdcall TTGetEmbeddedFontInfo(long ptr long ptr ptr ptr ptr)
@ stdcall TTGetEmbeddingType(ptr ptr)
@ stdcall TTIsEmbeddingEnabled(ptr ptr)
@ stdcall TTIsEmbeddingEnabledForFacename(str ptr)
@ stdcall TTLoadEmbeddedFont(ptr long ptr long ptr ptr ptr wstr str ptr)
@ stdcall TTRunValidationTests(ptr ptr)
@ stub _TTCharToUnicode@24
@ stub _TTDeleteEmbeddedFont@12
@ stub _TTEmbedFont@44
@ stub _TTEmbedFontFromFileA@52
@ stub _TTEnableEmbeddingForFacename@8
@ stub _TTGetEmbeddedFontInfo@28
@ stub _TTGetEmbeddingType@8
@ stub _TTIsEmbeddingEnabled@8
@ stub _TTIsEmbeddingEnabledForFacename@8
@ stub _TTLoadEmbeddedFont@40
@ stub _TTRunValidationTests@8
@ stdcall TTEmbedFontEx(ptr long long ptr ptr ptr ptr ptr long long ptr)
@ stdcall TTRunValidationTestsEx(ptr ptr)
@ stdcall TTGetNewFontName(ptr wstr long str long)
