/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/

#include "Geometry_shared.h"
#include "Worker.h"
#include "Helper/MathTools.h" //Min max
#include "GLApp/GLToolkit.h"
#include <cstring>
#include <cmath>
#include "GLApp/GLMatrix.h"
#include <tuple>
#include <Helper/GraphicsHelper.h>
#include <IntersectAABB_shared.h>

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#include "Interface/Interface.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLList.h"
#include "Interface/SmartSelection.h"
#include "Interface/FacetCoordinates.h"
#include "Interface/VertexCoordinates.h"
#include "Facet_shared.h"

#if defined(MOLFLOW)
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
extern SynRad*mApp;
#endif

#include "AABB.h"

std::array<float,4> applyGLColor4f(double value, float lower, float upper){

    static const std::vector<std::vector<float>> colorMap {
        {	0.6471	,	0.0000	,	0.1490	,	1.0000	},
        {	0.6547	,	0.0074	,	0.1492	,	1.0000	},
        {	0.6624	,	0.0148	,	0.1493	,	1.0000	},
        {	0.6701	,	0.0221	,	0.1495	,	1.0000	},
        {	0.6778	,	0.0295	,	0.1496	,	1.0000	},
        {	0.6855	,	0.0369	,	0.1498	,	1.0000	},
        {	0.6932	,	0.0443	,	0.1499	,	1.0000	},
        {	0.7009	,	0.0517	,	0.1501	,	1.0000	},
        {	0.7086	,	0.0591	,	0.1502	,	1.0000	},
        {	0.7163	,	0.0664	,	0.1504	,	1.0000	},
        {	0.7240	,	0.0738	,	0.1506	,	1.0000	},
        {	0.7316	,	0.0812	,	0.1507	,	1.0000	},
        {	0.7393	,	0.0886	,	0.1509	,	1.0000	},
        {	0.7470	,	0.0960	,	0.1510	,	1.0000	},
        {	0.7547	,	0.1033	,	0.1512	,	1.0000	},
        {	0.7624	,	0.1107	,	0.1513	,	1.0000	},
        {	0.7701	,	0.1181	,	0.1515	,	1.0000	},
        {	0.7778	,	0.1255	,	0.1516	,	1.0000	},
        {	0.7855	,	0.1329	,	0.1518	,	1.0000	},
        {	0.7932	,	0.1403	,	0.1519	,	1.0000	},
        {	0.8008	,	0.1476	,	0.1521	,	1.0000	},
        {	0.8085	,	0.1550	,	0.1522	,	1.0000	},
        {	0.8162	,	0.1624	,	0.1524	,	1.0000	},
        {	0.8239	,	0.1698	,	0.1526	,	1.0000	},
        {	0.8316	,	0.1772	,	0.1527	,	1.0000	},
        {	0.8393	,	0.1845	,	0.1529	,	1.0000	},
        {	0.8454	,	0.1929	,	0.1551	,	1.0000	},
        {	0.8498	,	0.2023	,	0.1594	,	1.0000	},
        {	0.8543	,	0.2117	,	0.1637	,	1.0000	},
        {	0.8587	,	0.2211	,	0.1680	,	1.0000	},
        {	0.8632	,	0.2304	,	0.1723	,	1.0000	},
        {	0.8677	,	0.2398	,	0.1766	,	1.0000	},
        {	0.8721	,	0.2492	,	0.1809	,	1.0000	},
        {	0.8766	,	0.2586	,	0.1852	,	1.0000	},
        {	0.8810	,	0.2680	,	0.1895	,	1.0000	},
        {	0.8855	,	0.2774	,	0.1938	,	1.0000	},
        {	0.8900	,	0.2867	,	0.1982	,	1.0000	},
        {	0.8944	,	0.2961	,	0.2025	,	1.0000	},
        {	0.8989	,	0.3055	,	0.2068	,	1.0000	},
        {	0.9033	,	0.3149	,	0.2111	,	1.0000	},
        {	0.9078	,	0.3243	,	0.2154	,	1.0000	},
        {	0.9123	,	0.3336	,	0.2197	,	1.0000	},
        {	0.9167	,	0.3430	,	0.2240	,	1.0000	},
        {	0.9212	,	0.3524	,	0.2283	,	1.0000	},
        {	0.9256	,	0.3618	,	0.2326	,	1.0000	},
        {	0.9301	,	0.3712	,	0.2369	,	1.0000	},
        {	0.9346	,	0.3805	,	0.2412	,	1.0000	},
        {	0.9390	,	0.3899	,	0.2455	,	1.0000	},
        {	0.9435	,	0.3993	,	0.2498	,	1.0000	},
        {	0.9479	,	0.4087	,	0.2541	,	1.0000	},
        {	0.9524	,	0.4181	,	0.2584	,	1.0000	},
        {	0.9569	,	0.4275	,	0.2627	,	1.0000	},
        {	0.9582	,	0.4374	,	0.2674	,	1.0000	},
        {	0.9596	,	0.4474	,	0.2720	,	1.0000	},
        {	0.9610	,	0.4574	,	0.2766	,	1.0000	},
        {	0.9624	,	0.4674	,	0.2812	,	1.0000	},
        {	0.9638	,	0.4774	,	0.2858	,	1.0000	},
        {	0.9652	,	0.4874	,	0.2904	,	1.0000	},
        {	0.9666	,	0.4974	,	0.2950	,	1.0000	},
        {	0.9679	,	0.5074	,	0.2997	,	1.0000	},
        {	0.9693	,	0.5174	,	0.3043	,	1.0000	},
        {	0.9707	,	0.5274	,	0.3089	,	1.0000	},
        {	0.9721	,	0.5374	,	0.3135	,	1.0000	},
        {	0.9735	,	0.5474	,	0.3181	,	1.0000	},
        {	0.9749	,	0.5574	,	0.3227	,	1.0000	},
        {	0.9762	,	0.5674	,	0.3273	,	1.0000	},
        {	0.9776	,	0.5774	,	0.3319	,	1.0000	},
        {	0.9790	,	0.5874	,	0.3366	,	1.0000	},
        {	0.9804	,	0.5974	,	0.3412	,	1.0000	},
        {	0.9818	,	0.6074	,	0.3458	,	1.0000	},
        {	0.9832	,	0.6174	,	0.3504	,	1.0000	},
        {	0.9845	,	0.6274	,	0.3550	,	1.0000	},
        {	0.9859	,	0.6374	,	0.3596	,	1.0000	},
        {	0.9873	,	0.6474	,	0.3642	,	1.0000	},
        {	0.9887	,	0.6574	,	0.3689	,	1.0000	},
        {	0.9901	,	0.6674	,	0.3735	,	1.0000	},
        {	0.9915	,	0.6774	,	0.3781	,	1.0000	},
        {	0.9922	,	0.6862	,	0.3840	,	1.0000	},
        {	0.9924	,	0.6939	,	0.3912	,	1.0000	},
        {	0.9925	,	0.7016	,	0.3985	,	1.0000	},
        {	0.9927	,	0.7093	,	0.4057	,	1.0000	},
        {	0.9928	,	0.7170	,	0.4129	,	1.0000	},
        {	0.9930	,	0.7246	,	0.4201	,	1.0000	},
        {	0.9932	,	0.7323	,	0.4274	,	1.0000	},
        {	0.9933	,	0.7400	,	0.4346	,	1.0000	},
        {	0.9935	,	0.7477	,	0.4418	,	1.0000	},
        {	0.9936	,	0.7554	,	0.4491	,	1.0000	},
        {	0.9938	,	0.7631	,	0.4563	,	1.0000	},
        {	0.9939	,	0.7708	,	0.4635	,	1.0000	},
        {	0.9941	,	0.7785	,	0.4707	,	1.0000	},
        {	0.9942	,	0.7862	,	0.4780	,	1.0000	},
        {	0.9944	,	0.7938	,	0.4852	,	1.0000	},
        {	0.9945	,	0.8015	,	0.4924	,	1.0000	},
        {	0.9947	,	0.8092	,	0.4997	,	1.0000	},
        {	0.9948	,	0.8169	,	0.5069	,	1.0000	},
        {	0.9950	,	0.8246	,	0.5141	,	1.0000	},
        {	0.9952	,	0.8323	,	0.5213	,	1.0000	},
        {	0.9953	,	0.8400	,	0.5286	,	1.0000	},
        {	0.9955	,	0.8477	,	0.5358	,	1.0000	},
        {	0.9956	,	0.8554	,	0.5430	,	1.0000	},
        {	0.9958	,	0.8631	,	0.5502	,	1.0000	},
        {	0.9959	,	0.8707	,	0.5575	,	1.0000	},
        {	0.9961	,	0.8784	,	0.5647	,	1.0000	},
        {	0.9962	,	0.8832	,	0.5719	,	1.0000	},
        {	0.9964	,	0.8880	,	0.5792	,	1.0000	},
        {	0.9965	,	0.8927	,	0.5864	,	1.0000	},
        {	0.9967	,	0.8975	,	0.5936	,	1.0000	},
        {	0.9968	,	0.9023	,	0.6008	,	1.0000	},
        {	0.9970	,	0.9070	,	0.6081	,	1.0000	},
        {	0.9972	,	0.9118	,	0.6153	,	1.0000	},
        {	0.9973	,	0.9166	,	0.6225	,	1.0000	},
        {	0.9975	,	0.9213	,	0.6298	,	1.0000	},
        {	0.9976	,	0.9261	,	0.6370	,	1.0000	},
        {	0.9978	,	0.9309	,	0.6442	,	1.0000	},
        {	0.9979	,	0.9356	,	0.6514	,	1.0000	},
        {	0.9981	,	0.9404	,	0.6587	,	1.0000	},
        {	0.9982	,	0.9452	,	0.6659	,	1.0000	},
        {	0.9984	,	0.9499	,	0.6731	,	1.0000	},
        {	0.9985	,	0.9547	,	0.6804	,	1.0000	},
        {	0.9987	,	0.9595	,	0.6876	,	1.0000	},
        {	0.9988	,	0.9642	,	0.6948	,	1.0000	},
        {	0.9990	,	0.9690	,	0.7020	,	1.0000	},
        {	0.9992	,	0.9738	,	0.7093	,	1.0000	},
        {	0.9993	,	0.9785	,	0.7165	,	1.0000	},
        {	0.9995	,	0.9833	,	0.7237	,	1.0000	},
        {	0.9996	,	0.9881	,	0.7309	,	1.0000	},
        {	0.9998	,	0.9928	,	0.7382	,	1.0000	},
        {	0.9999	,	0.9976	,	0.7454	,	1.0000	},
        {	0.9976	,	0.9991	,	0.7534	,	1.0000	},
        {	0.9928	,	0.9972	,	0.7622	,	1.0000	},
        {	0.9881	,	0.9954	,	0.7709	,	1.0000	},
        {	0.9833	,	0.9935	,	0.7797	,	1.0000	},
        {	0.9785	,	0.9917	,	0.7885	,	1.0000	},
        {	0.9738	,	0.9899	,	0.7972	,	1.0000	},
        {	0.9690	,	0.9880	,	0.8060	,	1.0000	},
        {	0.9642	,	0.9862	,	0.8148	,	1.0000	},
        {	0.9595	,	0.9843	,	0.8235	,	1.0000	},
        {	0.9547	,	0.9825	,	0.8323	,	1.0000	},
        {	0.9499	,	0.9806	,	0.8411	,	1.0000	},
        {	0.9452	,	0.9788	,	0.8498	,	1.0000	},
        {	0.9404	,	0.9769	,	0.8586	,	1.0000	},
        {	0.9356	,	0.9751	,	0.8674	,	1.0000	},
        {	0.9309	,	0.9732	,	0.8761	,	1.0000	},
        {	0.9261	,	0.9714	,	0.8849	,	1.0000	},
        {	0.9213	,	0.9696	,	0.8937	,	1.0000	},
        {	0.9166	,	0.9677	,	0.9024	,	1.0000	},
        {	0.9118	,	0.9659	,	0.9112	,	1.0000	},
        {	0.9070	,	0.9640	,	0.9200	,	1.0000	},
        {	0.9023	,	0.9622	,	0.9287	,	1.0000	},
        {	0.8975	,	0.9603	,	0.9375	,	1.0000	},
        {	0.8927	,	0.9585	,	0.9463	,	1.0000	},
        {	0.8880	,	0.9566	,	0.9550	,	1.0000	},
        {	0.8832	,	0.9548	,	0.9638	,	1.0000	},
        {	0.8784	,	0.9529	,	0.9725	,	1.0000	},
        {	0.8703	,	0.9489	,	0.9702	,	1.0000	},
        {	0.8621	,	0.9449	,	0.9679	,	1.0000	},
        {	0.8540	,	0.9409	,	0.9656	,	1.0000	},
        {	0.8458	,	0.9369	,	0.9633	,	1.0000	},
        {	0.8377	,	0.9329	,	0.9610	,	1.0000	},
        {	0.8295	,	0.9290	,	0.9587	,	1.0000	},
        {	0.8214	,	0.9250	,	0.9564	,	1.0000	},
        {	0.8132	,	0.9210	,	0.9541	,	1.0000	},
        {	0.8051	,	0.9170	,	0.9518	,	1.0000	},
        {	0.7969	,	0.9130	,	0.9495	,	1.0000	},
        {	0.7888	,	0.9090	,	0.9472	,	1.0000	},
        {	0.7806	,	0.9050	,	0.9449	,	1.0000	},
        {	0.7725	,	0.9010	,	0.9426	,	1.0000	},
        {	0.7643	,	0.8970	,	0.9403	,	1.0000	},
        {	0.7562	,	0.8930	,	0.9379	,	1.0000	},
        {	0.7480	,	0.8890	,	0.9356	,	1.0000	},
        {	0.7399	,	0.8850	,	0.9333	,	1.0000	},
        {	0.7317	,	0.8810	,	0.9310	,	1.0000	},
        {	0.7236	,	0.8770	,	0.9287	,	1.0000	},
        {	0.7154	,	0.8730	,	0.9264	,	1.0000	},
        {	0.7073	,	0.8690	,	0.9241	,	1.0000	},
        {	0.6991	,	0.8650	,	0.9218	,	1.0000	},
        {	0.6910	,	0.8610	,	0.9195	,	1.0000	},
        {	0.6828	,	0.8570	,	0.9172	,	1.0000	},
        {	0.6747	,	0.8530	,	0.9149	,	1.0000	},
        {	0.6664	,	0.8476	,	0.9119	,	1.0000	},
        {	0.6579	,	0.8408	,	0.9082	,	1.0000	},
        {	0.6494	,	0.8341	,	0.9045	,	1.0000	},
        {	0.6410	,	0.8273	,	0.9008	,	1.0000	},
        {	0.6325	,	0.8205	,	0.8971	,	1.0000	},
        {	0.6241	,	0.8138	,	0.8934	,	1.0000	},
        {	0.6156	,	0.8070	,	0.8897	,	1.0000	},
        {	0.6072	,	0.8002	,	0.8860	,	1.0000	},
        {	0.5987	,	0.7935	,	0.8824	,	1.0000	},
        {	0.5902	,	0.7867	,	0.8787	,	1.0000	},
        {	0.5818	,	0.7799	,	0.8750	,	1.0000	},
        {	0.5733	,	0.7732	,	0.8713	,	1.0000	},
        {	0.5649	,	0.7664	,	0.8676	,	1.0000	},
        {	0.5564	,	0.7596	,	0.8639	,	1.0000	},
        {	0.5479	,	0.7529	,	0.8602	,	1.0000	},
        {	0.5395	,	0.7461	,	0.8565	,	1.0000	},
        {	0.5310	,	0.7393	,	0.8528	,	1.0000	},
        {	0.5226	,	0.7326	,	0.8491	,	1.0000	},
        {	0.5141	,	0.7258	,	0.8454	,	1.0000	},
        {	0.5057	,	0.7190	,	0.8418	,	1.0000	},
        {	0.4972	,	0.7123	,	0.8381	,	1.0000	},
        {	0.4887	,	0.7055	,	0.8344	,	1.0000	},
        {	0.4803	,	0.6987	,	0.8307	,	1.0000	},
        {	0.4718	,	0.6920	,	0.8270	,	1.0000	},
        {	0.4634	,	0.6852	,	0.8233	,	1.0000	},
        {	0.4549	,	0.6784	,	0.8196	,	1.0000	},
        {	0.4477	,	0.6698	,	0.8151	,	1.0000	},
        {	0.4404	,	0.6612	,	0.8107	,	1.0000	},
        {	0.4332	,	0.6526	,	0.8062	,	1.0000	},
        {	0.4260	,	0.6440	,	0.8018	,	1.0000	},
        {	0.4188	,	0.6354	,	0.7973	,	1.0000	},
        {	0.4115	,	0.6268	,	0.7928	,	1.0000	},
        {	0.4043	,	0.6181	,	0.7884	,	1.0000	},
        {	0.3971	,	0.6095	,	0.7839	,	1.0000	},
        {	0.3899	,	0.6009	,	0.7795	,	1.0000	},
        {	0.3826	,	0.5923	,	0.7750	,	1.0000	},
        {	0.3754	,	0.5837	,	0.7705	,	1.0000	},
        {	0.3682	,	0.5751	,	0.7661	,	1.0000	},
        {	0.3609	,	0.5665	,	0.7616	,	1.0000	},
        {	0.3537	,	0.5579	,	0.7572	,	1.0000	},
        {	0.3465	,	0.5493	,	0.7527	,	1.0000	},
        {	0.3393	,	0.5406	,	0.7483	,	1.0000	},
        {	0.3320	,	0.5320	,	0.7438	,	1.0000	},
        {	0.3248	,	0.5234	,	0.7393	,	1.0000	},
        {	0.3176	,	0.5148	,	0.7349	,	1.0000	},
        {	0.3103	,	0.5062	,	0.7304	,	1.0000	},
        {	0.3031	,	0.4976	,	0.7260	,	1.0000	},
        {	0.2959	,	0.4890	,	0.7215	,	1.0000	},
        {	0.2887	,	0.4804	,	0.7170	,	1.0000	},
        {	0.2814	,	0.4717	,	0.7126	,	1.0000	},
        {	0.2742	,	0.4631	,	0.7081	,	1.0000	},
        {	0.2691	,	0.4540	,	0.7035	,	1.0000	},
        {	0.2660	,	0.4443	,	0.6987	,	1.0000	},
        {	0.2629	,	0.4346	,	0.6940	,	1.0000	},
        {	0.2598	,	0.4249	,	0.6892	,	1.0000	},
        {	0.2567	,	0.4152	,	0.6844	,	1.0000	},
        {	0.2537	,	0.4055	,	0.6797	,	1.0000	},
        {	0.2506	,	0.3958	,	0.6749	,	1.0000	},
        {	0.2475	,	0.3862	,	0.6701	,	1.0000	},
        {	0.2444	,	0.3765	,	0.6654	,	1.0000	},
        {	0.2414	,	0.3668	,	0.6606	,	1.0000	},
        {	0.2383	,	0.3571	,	0.6558	,	1.0000	},
        {	0.2352	,	0.3474	,	0.6511	,	1.0000	},
        {	0.2321	,	0.3377	,	0.6463	,	1.0000	},
        {	0.2291	,	0.3280	,	0.6415	,	1.0000	},
        {	0.2260	,	0.3183	,	0.6368	,	1.0000	},
        {	0.2229	,	0.3087	,	0.6320	,	1.0000	},
        {	0.2198	,	0.2990	,	0.6272	,	1.0000	},
        {	0.2168	,	0.2893	,	0.6225	,	1.0000	},
        {	0.2137	,	0.2796	,	0.6177	,	1.0000	},
        {	0.2106	,	0.2699	,	0.6129	,	1.0000	},
        {	0.2075	,	0.2602	,	0.6082	,	1.0000	},
        {	0.2045	,	0.2505	,	0.6034	,	1.0000	},
        {	0.2014	,	0.2408	,	0.5986	,	1.0000	},
        {	0.1983	,	0.2311	,	0.5938	,	1.0000	},
        {	0.1952	,	0.2215	,	0.5891	,	1.0000	},
        {	0.1922	,	0.2118	,	0.5843	,	1.0000	}
    };

    if(isnan(value)) return std::array<float,4>{1.0f, 0.0f, 1.0f, 1.0f};
    if(lower >= upper) return std::array<float,4>{1.0f, 0.0f, 1.0f, 1.0f};

    int low = 0;
    int up = 1;
    if(upper-lower > 0.0f){
        value = (value - lower) / (upper-lower);
    }
    int pos = std::min((int)colorMap.size()-1, (int)(value * colorMap.size()));
    if(pos < 0) return std::array<float,4>{1.0f, 0.0f, 1.0f, 1.0f};

    std::array<float,4> color{1.0f, 0.0f, 1.0f, 1.0f};
    color[0] = colorMap[pos][0];
    color[1] = colorMap[pos][1];
    color[2] = colorMap[pos][2];
    color[3] = colorMap[pos][3];
    return color;
}

std::array<float,4> applyGLColor4f(const std::vector<float>& cmap, double value, float lower, float upper){
    if(isnan(value)) return std::array<float,4>{1.0f, 0.0f, 1.0f, 1.0f};
    if(lower >= upper) return std::array<float,4>{1.0f, 0.0f, 1.0f, 1.0f};

    int low = 0;
    int up = 1;
    if(upper-lower > 0.0f){
        value = (value - lower) / (upper-lower);
    }
    int pos = std::min((int)(cmap.size() / 4)-1, (int)(value * (cmap.size() / 4)));
    if(pos < 0) return std::array<float,4>{1.0f, 0.0f, 1.0f, 1.0f};

    std::array<float,4> color{1.0f, 0.0f, 1.0f, 1.0f};
    color[0] = cmap[pos * 4];
    color[1] = cmap[pos * 4 + 1];
    color[2] = cmap[pos * 4 + 2];
    color[3] = cmap[pos * 4 + 3];
    return color;
}

void Geometry::SelectFacet(size_t facetId) {
	if (!isLoaded) return;
	InterfaceFacet *f = facets[facetId];
	f->selected = (viewStruct == -1) || (viewStruct == f->sh.superIdx) || (f->sh.superIdx == -1);
	if (!f->selected) f->UnselectElem();
	nbSelectedHist = 0;
	AddToSelectionHist(facetId);
}

void Geometry::SelectArea(int x1, int y1, int x2, int y2, bool clear, bool unselect, bool vertexBound, bool circularSelection) {

	// Select a set of facet according to a 2D bounding rectangle
	// (x1,y1) and (x2,y2) are in viewport coordinates

	float rx, ry, rz, rw, r2;
	int _x1, _y1, _x2, _y2;

	_x1 = Min(x1, x2);
	_x2 = Max(x1, x2);
	_y1 = Min(y1, y2);
	_y2 = Max(y1, y2);

	if (circularSelection) {
		r2 = pow((float)(x1 - x2), 2) + pow((float)(y1 - y2), 2);
	}

	GLfloat mProj[16];
	GLfloat mView[16];
	GLVIEWPORT g;

	glGetFloatv(GL_PROJECTION_MATRIX, mProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, mView);
	glGetIntegerv(GL_VIEWPORT, (GLint *)&g);

	GLMatrix proj; proj.LoadGL(mProj);
	GLMatrix view; view.LoadGL(mView);
	GLMatrix m; m.Multiply(&proj, &view);

	if (clear && !unselect) UnselectAll();
	nbSelectedHist = 0;
	int lastPaintedProgress = -1;
	char tmp[256];
	int paintStep = (int)((double)sh.nbFacet / 10.0);

	for (int i = 0; i < sh.nbFacet; i++) {
		if (sh.nbFacet > 5000) {
			if ((i - lastPaintedProgress) > paintStep) {
				lastPaintedProgress = i;;
				sprintf(tmp, "Facet search: %d%%", (int)(i*100.0 / (double)sh.nbFacet));
				mApp->SetFacetSearchPrg(true, tmp);
			}
		}
		InterfaceFacet *f = facets[i];
		if (viewStruct == -1 || f->sh.superIdx == viewStruct || f->sh.superIdx == -1) {

			size_t nb = facets[i]->sh.nbIndex;
			bool isInside = true;
			size_t j = 0;
			bool hasSelectedVertex = false;
			while (j < nb && isInside) {

				size_t idx = f->indices[j];
				m.TransfomVec((float)vertices3[idx].x, (float)vertices3[idx].y, (float)vertices3[idx].z, 1.0f,
					&rx, &ry, &rz, &rw);

				if (rw > 0.0f) {
					int xe = (int)(((rx / rw) + 1.0f) * (float)g.width / 2.0f);
					int ye = (int)(((-ry / rw) + 1.0f) * (float)g.height / 2.0f);
					if (!circularSelection)
						isInside = (xe >= _x1) && (xe <= _x2) && (ye >= _y1) && (ye <= _y2);
					else //circular selection
						isInside = (pow((float)(xe - x1), 2) + pow((float)(ye - y1), 2)) <= r2;
					if (vertices3[idx].selected) hasSelectedVertex = true;
				}
				else {

					isInside = false;
				}
				j++;

			}

			if (isInside && (!vertexBound || hasSelectedVertex)) {
				if (!unselect) {
					f->selected = !unselect;
				}
				else {

					f->selected = !unselect;
				}
			}

		}
	}
	mApp->SetFacetSearchPrg(false, NULL);
	UpdateSelection();
}

void Geometry::Select(int x, int y, bool clear, bool unselect, bool vertexBound, int width, int height) {

	int i;
	if (!isLoaded) return;

	// Select a facet on a mouse click in 3D perspectivce view 
	// (x,y) are in screen coordinates
	// TODO: Handle clipped polygon

	// Check intersection of the facet and a "perspective ray"
	std::vector<int> screenXCoords(sh.nbVertex);
	std::vector<int> screenYCoords(sh.nbVertex);

	// Transform points to screen coordinates
	std::vector<bool> ok(sh.nbVertex);
	std::vector<bool> onScreen(sh.nbVertex);
	for (i = 0; i < sh.nbVertex; i++) {//here we could speed up by choosing visible vertices only?
		if (auto screenCoords = GLToolkit::Get2DScreenCoord(vertices3[i])) {
			ok[i] = true;
			std::tie(screenXCoords[i],screenYCoords[i]) = *screenCoords;
			onScreen[i] = screenXCoords[i] >= 0 && screenYCoords[i] >= 0 && screenXCoords[i] <= width && screenYCoords[i] <= height;
		}
		else {
			ok[i] = false;
			//onScreen[i] = false;
		}
	}

	// Check facets
	bool found = false;
	bool clipped;
	bool hasVertexOnScreen;
	bool hasSelectedVertex;
	i = 0;
	char tmp[256];
	int lastFound = -1;
	int lastPaintedProgress = -1;
	int paintStep = (int)((double)sh.nbFacet / 10.0);

	while (i < sh.nbFacet && !found) {
		if (sh.nbFacet > 5000) {
			if ((i - lastPaintedProgress) > paintStep) {
				lastPaintedProgress = i;;
				sprintf(tmp, "Facet search: %d%%", (int)(i*100.0 / (double)sh.nbFacet));
				mApp->SetFacetSearchPrg(true, tmp);
			}
		}
		if (viewStruct == -1 || facets[i]->sh.superIdx == viewStruct || facets[i]->sh.superIdx == -1) {

			clipped = false;
			hasVertexOnScreen = false;
			hasSelectedVertex = false;
			// Build array of 2D points
			std::vector<Vector2d> v(facets[i]->indices.size());

			for (int j = 0; j < facets[i]->indices.size() && !clipped; j++) {
				size_t idx = facets[i]->indices[j];
				if (ok[idx]) {
					v[j] = Vector2d((double)screenXCoords[idx],(double)screenYCoords[idx]);
					if (onScreen[idx]) hasVertexOnScreen = true;
				}
				else {
					clipped = true;
				}
			}
			if (vertexBound) { //CAPS LOCK on, select facets onyl with at least one seleted vertex
				for (size_t j = 0; j < facets[i]->indices.size() && (!hasSelectedVertex); j++) {
					size_t idx = facets[i]->indices[j];
					if (vertices3[idx].selected) hasSelectedVertex = true;
				}
			}

			if (!clipped && hasVertexOnScreen && (!vertexBound || hasSelectedVertex)) {

				found = IsInPoly((double)x,(double)y, v);

				if (found) {
					if (unselect) {
						if (!mApp->smartSelection || !mApp->smartSelection->IsSmartSelection()) {
							facets[i]->selected = false;
							found = false; //Continue looking for facets
						}
						else { //Smart selection
							double maxAngleDiff = mApp->smartSelection->GetMaxAngle();
							std::vector<size_t> connectedFacets;
							mApp->SetFacetSearchPrg(true, "Smart selecting...");
							if (maxAngleDiff >= 0.0) connectedFacets = GetConnectedFacets(i, maxAngleDiff);
							for (auto& ind : connectedFacets)
								facets[ind]->selected = false;
							mApp->SetFacetSearchPrg(false, "");
						}
					} //end unselect

					if (AlreadySelected(i)) {

						lastFound = i;
						found = false; //Continue looking for facets

					}
				} //end found

			}

		}

		if (!found) i++;

	}
	mApp->SetFacetSearchPrg(false, "");
	if (clear && !unselect) UnselectAll();

	if (!found && lastFound >= 0) {
		if (!unselect) {
			// Restart
			nbSelectedHist = 0;
			AddToSelectionHist(lastFound);
		}
		facets[lastFound]->selected = !unselect;
		if (!unselect) mApp->facetList->ScrollToVisible(lastFound, 0, true); //scroll to selected facet
	}
	else {

		if (found) {
			if (!unselect) AddToSelectionHist(i);
			if (!mApp->smartSelection || !mApp->smartSelection->IsSmartSelection()) {
				facets[i]->selected = !unselect;
			}
			else { //Smart selection
				double maxAngleDiff = mApp->smartSelection->GetMaxAngle();
				std::vector<size_t> connectedFacets;
				mApp->SetFacetSearchPrg(true, "Smart selecting...");
				if (maxAngleDiff >= 0.0) connectedFacets = GetConnectedFacets(i, maxAngleDiff);
				for (auto& ind : connectedFacets)
					facets[ind]->selected = !unselect;
				mApp->SetFacetSearchPrg(false, "");
			}
			if (!unselect) mApp->facetList->ScrollToVisible(i, 0, true); //scroll to selected facet
		}
		else {

			nbSelectedHist = 0;
		}
	}
	UpdateSelection();

}

void Geometry::SelectVertex(int vertexId) {
	//isVertexSelected[vertexId] = (viewStruct==-1) || (viewStruct==f->wp.superIdx);
	//here we should look through facets if vertex is member of any
	//if( !f->selected ) f->UnselectElem();
	if (!isLoaded) return;
	vertices3[vertexId].selected = true;
}

void Geometry::SelectVertex(int x1, int y1, int x2, int y2, bool shiftDown, bool ctrlDown, bool circularSelection, bool facetBound) {

	// Select a set of vertices according to a 2D bounding rectangle
	// (x1,y1) and (x2,y2) are in viewport coordinates

	float rx, ry, rz, rw, r2;
	int _x1, _y1, _x2, _y2;

	_x1 = Min(x1, x2);
	_x2 = Max(x1, x2);
	_y1 = Min(y1, y2);
	_y2 = Max(y1, y2);

	if (circularSelection) {
		r2 = pow((float)(x1 - x2), 2) + pow((float)(y1 - y2), 2);
	}

	GLfloat mProj[16];
	GLfloat mView[16];
	GLVIEWPORT g;

	glGetFloatv(GL_PROJECTION_MATRIX, mProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, mView);
	glGetIntegerv(GL_VIEWPORT, (GLint *)&g);

	GLMatrix proj; proj.LoadGL(mProj);
	GLMatrix view; view.LoadGL(mView);
	GLMatrix m; m.Multiply(&proj, &view);

	if (!ctrlDown && !shiftDown) {
		UnselectAllVertex(); EmptySelectedVertexList();
		//nbSelectedHistVertex = 0;
	}

	std::vector<bool> selectedFacetsVertices;
	if (facetBound) selectedFacetsVertices = GetVertexBelongsToSelectedFacet();

	for (int i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		Vector3d *v = GetVertex(i);
		//if(viewStruct==-1 || f->wp.superIdx==viewStruct) {
		if (true) {

			bool isInside;
			int idx = i;
			m.TransfomVec((float)vertices3[idx].x, (float)vertices3[idx].y, (float)vertices3[idx].z, 1.0f,
				&rx, &ry, &rz, &rw);

			if (rw > 0.0f) {
				int xe = (int)(((rx / rw) + 1.0f) * (float)g.width / 2.0f);
				int ye = (int)(((-ry / rw) + 1.0f) * (float)g.height / 2.0f);
				if (!circularSelection)
					isInside = (xe >= _x1) && (xe <= _x2) && (ye >= _y1) && (ye <= _y2);
				else //circular selection
					isInside = (pow((float)(xe - x1), 2) + pow((float)(ye - y1), 2)) <= r2;
			}
			else {

				isInside = false;
			}

			if (isInside) {
				vertices3[i].selected = !ctrlDown;
				if (ctrlDown) RemoveFromSelectedVertexList(i);
				else {
					AddToSelectedVertexList(i);
					if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateId(i);
				}
			}
		}
	}

	//UpdateSelectionVertex();
	if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
}

void Geometry::SelectVertex(int x, int y, bool shiftDown, bool ctrlDown, bool facetBound) {
	int i;
	if (!isLoaded) return;

	// Select a vertex on a mouse click in 3D perspectivce view 
	// (x,y) are in screen coordinates
	// TODO: Handle clipped polygon

	// Check intersection of the facet and a "perspective ray"
	std::vector<int> allXe(sh.nbVertex);
	std::vector<int> allYe(sh.nbVertex);
	std::vector<bool> ok(sh.nbVertex);

	std::vector<bool> selectedFacetsVertices;
	if (facetBound) selectedFacetsVertices = GetVertexBelongsToSelectedFacet();

	// Transform points to screen coordinates
	for (i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		if (auto screenCoords = GLToolkit::Get2DScreenCoord(vertices3[i])) {
			ok[i] = true;
			std::tie(allXe[i], allYe[i]) = *screenCoords;
		}
		else {
			ok[i] = false;
		}
	}

	//Get Closest Point to click
	double minDist = 9999;
	double distance;
	int minId = -1;
	for (i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		if (ok[i] && !(allXe[i] < 0) && !(allYe[i] < 0)) { //calculate only for points on screen
			distance = pow((double)(allXe[i] - x), 2) + pow((double)(allYe[i] - y), 2);
			if (distance < minDist) {
				minDist = distance;
				minId = i;
			}
		}
	}

	if (!ctrlDown && !shiftDown) {
		UnselectAllVertex(); EmptySelectedVertexList();
		//nbSelectedHistVertex = 0;
	}

	if (minDist < 250.0) {
		vertices3[minId].selected = !ctrlDown;
		if (ctrlDown) RemoveFromSelectedVertexList(minId);
		else {
			AddToSelectedVertexList(minId);
			if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateId(minId);
			//nbSelectedHistVertex++;
		}
	}

	//UpdateSelection();
	if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
}

void Geometry::AddToSelectionHist(size_t f) {

	if (nbSelectedHist < SEL_HISTORY) {
		selectHist[nbSelectedHist] = f;
		nbSelectedHist++;
	}

}

bool Geometry::AlreadySelected(size_t f) {

	// Check if the facet has already been selected
	bool found = false;
	size_t i = 0;
	while (!found && i < nbSelectedHist) {
		found = (selectHist[i] == f);
		if (!found) i++;
	}
	return found;

}

void Geometry::SelectAll() {
	for (int i = 0; i < sh.nbFacet; i++)
		SelectFacet(i);
	UpdateSelection();
}

void Geometry::EmptySelectedVertexList() {
	selectedVertexList_ordered.clear();
}

void Geometry::RemoveFromSelectedVertexList(size_t vertexId) {
	selectedVertexList_ordered.erase(std::remove(selectedVertexList_ordered.begin(), selectedVertexList_ordered.end(), vertexId), selectedVertexList_ordered.end());
}

void Geometry::AddToSelectedVertexList(size_t vertexId) {
	selectedVertexList_ordered.push_back(vertexId);
}

void Geometry::SelectAllVertex() {
	for (int i = 0; i < sh.nbVertex; i++)
		SelectVertex(i);
	//UpdateSelectionVertex();
}

size_t Geometry::GetNbSelectedVertex() {
	size_t nbSelectedVertex = 0;
	for (int i = 0; i < sh.nbVertex; i++) {
		if (vertices3[i].selected) nbSelectedVertex++;
	}
	return nbSelectedVertex;
}

void Geometry::UnselectAll() {
	for (int i = 0; i < sh.nbFacet; i++) {
		facets[i]->selected = false;
		facets[i]->UnselectElem();
	}
	UpdateSelection();
}

void Geometry::UnselectAllVertex() {
	for (int i = 0; i < sh.nbVertex; i++) {
		vertices3[i].selected = false;
		//facets[i]->UnselectElem(); //what is this?
	}
	//UpdateSelectionVertex();
}

std::vector<size_t> Geometry::GetSelectedVertices()
{
	std::vector<size_t> sel;
	for (size_t i = 0; i < sh.nbVertex; i++)
		if (vertices3[i].selected) sel.push_back(i);
	return sel;
}

void Geometry::DrawFacet(InterfaceFacet *f, bool offset, bool showHidden, bool selOffset) {

	// Render a facet (wireframe)
	size_t nb = f->sh.nbIndex;
	size_t i1;

	if (offset) {

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		if (selOffset) {
			glPolygonOffset(0.0f, 1.0f);
		}
		else {

			glPolygonOffset(0.0f, 5.0f);
		}
		glBegin(GL_POLYGON);
		for (size_t j = 0; j < nb; j++) {
			i1 = f->indices[j];
			glEdgeFlag(f->visible[j] || showHidden);
			glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
		}
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);

	}
	else {

		if (nb < 8) {
			// No hole possible
			glBegin(GL_LINE_LOOP);
			for (size_t j = 0; j < nb; j++) {
				i1 = f->indices[j];
				glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
			}
			glEnd();
		}
		else {

			glBegin(GL_LINES);
			size_t i1, i2, j;
			for (j = 0; j < nb - 1; j++) {
				if (f->visible[j] || showHidden) {
					i1 = f->indices[j];
					i2 = f->indices[j + 1];
					glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
					glVertex3d(vertices3[i2].x, vertices3[i2].y, vertices3[i2].z);
				}
			}
			// Last segment
			if (f->visible[j] || showHidden) {
				i1 = f->indices[j];
				i2 = f->indices[0];
				glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
				glVertex3d(vertices3[i2].x, vertices3[i2].y, vertices3[i2].z);
			}
			glEnd();
		}
	}

}

void Geometry::DrawAABBNode(const std::shared_ptr<RTPrimitive> prim) {
    if(dynamic_cast<BVHAccel*>(prim.get())){
        DrawAABBNode(*dynamic_cast<BVHAccel*>(prim.get()));
    }
    else if(dynamic_cast<KdTreeAccel*>(prim.get())){
        DrawAABBNode(*dynamic_cast<KdTreeAccel*>(prim.get()));
    }
}

void Geometry::DrawAABBNode(const KdTreeAccel &kd) {
    if (!kd.nodes)
        return;

    const KdAccelNode *node = &kd.nodes[0];
    //DrawAABBNode(node, kd.bounds, 0, 0);
    DrawAABBPlane(node, kd.bounds, 0, 0, false);
    if(mApp->aabbVisu.selectedNode > -1)
        DrawAABBPlane(node, kd.bounds, 0, 0, true);
}

void Geometry::DrawAABBNode(const KdAccelNode *lnode, AxisAlignedBoundingBox bb, int currentNodeIndex, int level) {


    const KdAccelNode *node = &lnode[currentNodeIndex];

    const int max_level = (mApp->aabbVisu.showLevelAABB[1] == -1) ? 64 : mApp->aabbVisu.showLevelAABB[1];
    if(node->IsLeaf() && !mApp->aabbVisu.showAABBLeaves) return;

    if(mApp->aabbVisu.showLevelAABB[0] == -1 || ((mApp->aabbVisu.showLevelAABB[0] <= level) && (level <= mApp->aabbVisu.showLevelAABB[1]))) {
        auto delta = this->bb.max - this->bb.min;
        auto bbox = bb;
        if(mApp->aabbVisu.boxExpansion)
            bbox.Expand((mApp->aabbVisu.reverseExpansion ? -0.0002 : 0.0002)*delta*level);

        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        if (mApp->antiAliasing) {
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
        }


        // begin transparent sides
        if(mApp->aabbVisu.selectedNode == currentNodeIndex) {
            glColor4f(
                    0.9f ,
                    0.2f ,
                    0.2f ,
                    0.5f);
        }
        else if(mApp->aabbVisu.sameColor){
            glColor4f(
                    (1.0f / (1.0f + ((int) (level / 3.0f)) * 0.1f)) ,
                    (1.0f / (1.0f + ((int) (level / 3.0f)) * 0.1f)) ,
                    (1.0f / (1.0f + ((int) (level / 3.0f)) * 0.1f)) ,
                    std::min(1.0f, ((int) (level / 3.0f) * 0.02f) + mApp->aabbVisu.alpha));
        }
        else {
            glColor4f(
                    (level % 3 == 0) ? (1.0f / (1.0f + ((int) (level / 3.0f)) * 0.1f)) : 0.2f,
                    (level % 3 == 1) ? (1.0f / (1.0f + ((int) (level / 3.0f)) * 0.1f)) : 0.2f,
                    (level % 3 == 2) ? (1.0f / (1.0f + ((int) (level / 3.0f)) * 0.1f)) : 0.2f,
                    std::min(1.0f, ((int) (level / 3.0f) * 0.02f) + mApp->aabbVisu.alpha));
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();


        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();
        // end transparent sides

        glPointSize(std::max(1.0f, 15.0f - level * 3.0f));
        if(mApp->aabbVisu.selectedNode == currentNodeIndex) {
            glColor4f(
                    0.9f ,
                    0.2f ,
                    0.2f ,
                    0.5f);
        }
        else if(mApp->aabbVisu.sameColor){
            glColor4f(
                    (1.0f / (1.0f + level * 0.5)),
                    (1.0f / (1.0f + level * 0.5)),
                    (1.0f / (1.0f + level * 0.5)),
                    std::min(1.0f, (level * 0.02f) + mApp->aabbVisu.alpha));
        }
        else {
            glColor4f(
                    (level % 3 == 0) ? (1.0f / (1.0f + level * 0.5)) : 0.1f,
                    (level % 3 == 1) ? (1.0f / (1.0f + (level - 1) * 0.5)) : 0.1f,
                    (level % 3 == 2) ? (1.0f / (1.0f + (level - 2) * 0.5)) : 0.1f,
                    std::min(1.0f, (level * 0.02f) + mApp->aabbVisu.alpha));
        }
        glBegin(GL_POINTS);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        if(mApp->aabbVisu.selectedNode == currentNodeIndex) {
            glColor4f(
                    0.9f ,
                    0.2f ,
                    0.2f ,
                    0.5f);
        }
        else if(mApp->aabbVisu.sameColor){
            glColor4f(
                    (1.0f / (1.0f + level * 0.5)),
                    (1.0f / (1.0f + level * 0.5)),
                    (1.0f / (1.0f + level * 0.5)),
                    std::min(1.0f, (level * 0.05f) + 0.4f));
        }
        else {
            glColor4f(
                    (level % 3 == 0) ? (1.0f / (1.0f + level * 0.5)) : 0.1f,
                    (level % 3 == 1) ? (1.0f / (1.0f + (level - 1) * 0.5)) : 0.1f,
                    (level % 3 == 2) ? (1.0f / (1.0f + (level - 2) * 0.5)) : 0.1f,
                    std::min(1.0f, (level * 0.05f) + 0.4f));
        }
        glLineWidth(3.0f);

        glPushAttrib(GL_LINE_BIT);
        glLineWidth(std::max(1.0f, 10.0f - level * 3.0f));
        if(mApp->aabbVisu.selectedNode == currentNodeIndex) {
            glColor4f(
                    0.9f ,
                    0.2f ,
                    0.2f ,
                    0.5f);
        }
        else if(mApp->aabbVisu.sameColor){
            glColor4f(
                    (1.0f / (1.0f + (level) * 0.5)),(1.0f / (1.0f + (level) * 0.5)),(1.0f / (1.0f + (level) * 0.5)),
                    std::min(1.0f, (level * 0.5f) + 0.1f));
        }
        else {
            glColor4f(
                    (level % 3 == 0) ? (1.0f / (1.0f + (level) * 0.5)) : 0.2f,
                    (level % 3 == 1) ? (1.0f / (1.0f + (level - 1) * 0.5)) : 0.2f,
                    (level % 3 == 2) ? (1.0f / (1.0f + (level - 2) * 0.5)) : 0.2f,
                    std::min(1.0f, (level * 0.5f) + 0.1f));
        }
        glBegin(GL_LINE_LOOP);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();

        glLineStipple(1, 0x0101);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_STRIP);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        glDisable(GL_LINE_STIPPLE);

        glPopAttrib();
        //glLineWidth(1.0f);

        if (mApp->antiAliasing) {
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
        }
        glDepthMask(GL_TRUE);
    }

    if(level < max_level && !node->IsLeaf()) {
        int axis = node->SplitAxis();
        double plane = node->SplitPos();

        auto bbl = bb.Split(axis, plane, true);
        auto bbr = bb.Split(axis, plane, false);
        if(mApp->aabbVisu.showBranchSide[0]) DrawAABBNode(lnode, bbl, currentNodeIndex + 1, level+1);
        if(mApp->aabbVisu.showBranchSide[1]) DrawAABBNode(lnode, bbr, node->AboveChild(), level+1);
    }
}

void Geometry::DrawAABBPlane(const KdAccelNode *lnode, AxisAlignedBoundingBox bb, int currentNodeIndex, int level,
                             bool selection) {


    const KdAccelNode *node = &lnode[currentNodeIndex];

    const int max_level = (mApp->aabbVisu.showLevelAABB[1] == -1) ? 64 : mApp->aabbVisu.showLevelAABB[1];
    if(node->IsLeaf() && !mApp->aabbVisu.showAABBLeaves) return;

    if((!mApp->aabbVisu.travStep || (mApp->aabbVisu.travStep && node->nPrims > 0)) && ((!selection && mApp->aabbVisu.selectedNode != currentNodeIndex) || (selection && mApp->aabbVisu.selectedNode == currentNodeIndex)) && (mApp->aabbVisu.showLevelAABB[0] == -1 || ((mApp->aabbVisu.showLevelAABB[0] <= level) && (level <= mApp->aabbVisu.showLevelAABB[1])))) {
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        if (mApp->antiAliasing) {
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
        }

        float rate = 0.0f;
        std::array<float,4> color{0.0f};
        if(mApp->aabbVisu.travStep && node->nPrims > 0){

        }
        else if(mApp->aabbVisu.showStats){
            rate = (!mApp->aabbVisu.rateVector->empty() || mApp->aabbVisu.rateVector->size() <= currentNodeIndex) ? mApp->aabbVisu.rateVector->at(currentNodeIndex) : 0.0f;
            if(rate >= mApp->aabbVisu.trimByProb[0] && rate <= mApp->aabbVisu.trimByProb[1]) {
                color = (mApp->aabbVisu.colorMap.empty()) ? applyGLColor4f(rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]) : applyGLColor4f(mApp->aabbVisu.colorMap, rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]);
            }
        }

        if(selection && mApp->aabbVisu.selectedNode == currentNodeIndex) {
            color = {0.9f ,
                     0.9f ,
                     0.2f ,
                     0.5f};
        }
        else if(mApp->aabbVisu.showStats){

        }
        else if(mApp->aabbVisu.sameColor){
            color = {0.6f ,
                     0.6f ,
                     std::max(0.7f, (1.0f / (1.0f + level * 0.5f))) ,
                     std::min(1.0f, (level * 0.033f) + mApp->aabbVisu.alpha)};
        }
        else {
            int minLevel=0, maxLevel=30;
            if(mApp->aabbVisu.showLevelAABB[0]>-1){
                minLevel = mApp->aabbVisu.showLevelAABB[0];
            }
            if(mApp->aabbVisu.showLevelAABB[1]>-1){
                maxLevel = mApp->aabbVisu.showLevelAABB[1];
            }
            color = (mApp->aabbVisu.colorMap.empty()) ? applyGLColor4f(level, minLevel, maxLevel) : applyGLColor4f(mApp->aabbVisu.colorMap, level, minLevel, maxLevel);
        }

        auto delta = this->bb.max - this->bb.min;
        auto bbox = bb;
        if(mApp->aabbVisu.boxExpansion)
            bbox.Expand((mApp->aabbVisu.reverseExpansion ? -0.002 : 0.002)*delta*level);

        glPointSize(std::max(1.0f, 5.0f - level * 0.5f));
        glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 1.2f);

        glBegin(GL_POINTS);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);

        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();


        // draw full box for selected node
        if(mApp->aabbVisu.onlyBorder || (selection && mApp->aabbVisu.selectedNode == currentNodeIndex)) {
            /*if(mApp->aabbVisu.onlyBorder) {
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ZERO);
            }*/

            // only draw fille box for selection
            if(!mApp->aabbVisu.onlyBorder) {
                glColor4f(color[0], color[1], color[2], color[3] * mApp->aabbVisu.alpha * 0.2f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glBegin(GL_POLYGON);
                glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
                glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
                glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
                glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
                glEnd();

                glBegin(GL_POLYGON);
                glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
                glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
                glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
                glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
                glEnd();

                glBegin(GL_POLYGON);
                glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
                glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
                glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
                glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
                glEnd();


                glBegin(GL_POLYGON);
                glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
                glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
                glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
                glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
                glEnd();

                glBegin(GL_POLYGON);
                glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
                glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
                glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
                glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
                glEnd();

                glBegin(GL_POLYGON);
                glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
                glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
                glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
                glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
                glEnd();
            }
            // end transparent sides

            // also draw edges
            glPushAttrib(GL_LINE_BIT);

            glLineWidth(std::max(1.0f, 5.0f - level * 0.5f));

            glEnable(GL_BLEND);
            if(mApp->aabbVisu.onlyBorder && !selection) {
                glEnable(GL_DEPTH_TEST);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
            }
            else
                glBlendFunc(GL_ONE, GL_ZERO);
            glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 0.2f);
            glBegin(GL_LINE_LOOP);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
            glEnd();

            glBegin(GL_LINE_LOOP);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
            glEnd();
            glBegin(GL_LINES);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
            glEnd();
            glBegin(GL_LINES);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glEnd();
            glBegin(GL_LINES);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
            glEnd();

            glLineStipple(1, 0x0101);
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINE_STRIP);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glEnd();
            glDisable(GL_LINE_STIPPLE);

            glPopAttrib();
        }

        if(!node->IsLeaf() && !mApp->aabbVisu.onlyBorder) {
            int axis = node->SplitAxis();
            double plane = node->SplitPos();

            bbox = bb;
            bbox.min[axis] = plane;
            bbox.max[axis] = plane;
            if(mApp->aabbVisu.boxExpansion)
                bbox.Expand((mApp->aabbVisu.reverseExpansion ? -0.002 : 0.002) * delta[axis], axis);

            Vector3d a = bbox.min;
            Vector3d b = bbox.min;
            Vector3d c = bbox.max;
            Vector3d d = bbox.max;

            const int axisSwap = (axis + 2) % 3;
            b[axisSwap] = bbox.max[axisSwap];
            d[axisSwap] = bbox.min[axisSwap];

            // begin transparent sides
            if(selection && mApp->aabbVisu.selectedNode == currentNodeIndex) {
                glColor4f(color[0], color[1], color[2], std::max(color[3] * mApp->aabbVisu.alpha * 1.2f, 0.45f));
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else{
                glColor4f(color[0], color[1], color[2], color[3] * mApp->aabbVisu.alpha);
            }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBegin(GL_POLYGON);
            glVertex3d(a.x, a.y, a.z);
            glVertex3d(b.x, b.y, b.z);
            glVertex3d(c.x, c.y, c.z);
            glVertex3d(d.x, d.y, d.z);
            glEnd();
            // end transparent sides
        }

        if (mApp->antiAliasing) {
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
        }
        glDepthMask(GL_TRUE);
    }

    if(level < max_level && !node->IsLeaf()) {
        int axis = node->SplitAxis();
        double plane = node->SplitPos();

        auto bbl = bb.Split(axis, plane, true);
        auto bbr = bb.Split(axis, plane, false);
        if(mApp->aabbVisu.showBranchSide[0]) DrawAABBPlane(lnode, bbl, currentNodeIndex + 1, level + 1, selection);
        if(mApp->aabbVisu.showBranchSide[1]) DrawAABBPlane(lnode, bbr, node->AboveChild(), level + 1, selection);
    }
}
/*

void Geometry::DrawAABBPlaneLine(const KdAccelNode *lnode, AxisAlignedBoundingBox bb, int currentNodeIndex, int level,
                             bool selection) {


    const KdAccelNode *node = &lnode[currentNodeIndex];

    const int max_level = (mApp->aabbVisu.showLevelAABB[1] == -1) ? 64 : mApp->aabbVisu.showLevelAABB[1];
    if(node->IsLeaf() && !mApp->aabbVisu.showAABBLeaves) return;

    if((!mApp->aabbVisu.travStep || (mApp->aabbVisu.travStep && node->nPrims > 0)) && ((!selection && mApp->aabbVisu.selectedNode != currentNodeIndex) || (selection && mApp->aabbVisu.selectedNode == currentNodeIndex)) && (mApp->aabbVisu.showLevelAABB[0] == -1 || ((mApp->aabbVisu.showLevelAABB[0] <= level) && (level <= mApp->aabbVisu.showLevelAABB[1])))) {
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        if (mApp->antiAliasing) {
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
        }

        float rate = 0.0f;
        std::array<float,4> color{0.0f};
        if(mApp->aabbVisu.travStep && node->nPrims > 0){

        }
        else if(mApp->aabbVisu.showStats){
            rate = (!mApp->aabbVisu.rateVector->empty() || mApp->aabbVisu.rateVector->size() <= currentNodeIndex) ? mApp->aabbVisu.rateVector->at(currentNodeIndex) : 0.0f;
            if(rate >= mApp->aabbVisu.trimByProb[0] && rate <= mApp->aabbVisu.trimByProb[1]) {
                color = (mApp->aabbVisu.colorMap.empty()) ? applyGLColor4f(rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]) : applyGLColor4f(mApp->aabbVisu.colorMap, rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]);
            }
        }

        if(selection && mApp->aabbVisu.selectedNode == currentNodeIndex) {
            color = {0.9f ,
                     0.9f ,
                     0.2f ,
                     0.5f};
        }
        else if(mApp->aabbVisu.showStats){

        }
        else if(mApp->aabbVisu.sameColor){
            color = {0.6f ,
                     0.6f ,
                     std::max(0.7f, (1.0f / (1.0f + level * 0.5f))) ,
                     std::min(1.0f, (level * 0.033f) + mApp->aabbVisu.alpha)};
        }
        else {
            int minLevel=0, maxLevel=30;
            if(mApp->aabbVisu.showLevelAABB[0]>-1){
                minLevel = mApp->aabbVisu.showLevelAABB[0];
            }
            if(mApp->aabbVisu.showLevelAABB[1]>-1){
                maxLevel = mApp->aabbVisu.showLevelAABB[1];
            }
            color = (mApp->aabbVisu.colorMap.empty()) ? applyGLColor4f(level, minLevel, maxLevel) : applyGLColor4f(mApp->aabbVisu.colorMap, level, minLevel, maxLevel);
        }

        auto delta = this->bb.max - this->bb.min;
        auto bbox = bb;
        if(mApp->aabbVisu.boxExpansion)
            bbox.Expand((mApp->aabbVisu.reverseExpansion ? -0.0002 : 0.0002)*delta*level);

        glPointSize(std::max(1.0f, 15.0f - level * 3.0f));
        glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 1.2f);

        glBegin(GL_POINTS);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();


        // draw full box for selected node
        if(selection && mApp->aabbVisu.selectedNode == currentNodeIndex) {
            //glDisable(GL_DEPTH_TEST);
            //glEnable(GL_BLEND);
            //glBlendFunc(GL_ONE, GL_ZERO);
            glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 0.2f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBegin(GL_POLYGON);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
            glEnd();

            glBegin(GL_POLYGON);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
            glEnd();

            glBegin(GL_POLYGON);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
            glEnd();


            glBegin(GL_POLYGON);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
            glEnd();

            glBegin(GL_POLYGON);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
            glEnd();

            glBegin(GL_POLYGON);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
            glEnd();
            // end transparent sides

            // also draw edges
            glPushAttrib(GL_LINE_BIT);

            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ZERO);
            glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 0.2f);
            glBegin(GL_LINE_LOOP);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
            glEnd();

            glBegin(GL_LINE_LOOP);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
            glEnd();
            glBegin(GL_LINES);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
            glEnd();
            glBegin(GL_LINES);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glEnd();
            glBegin(GL_LINES);
            glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
            glEnd();

            glLineStipple(1, 0x0101);
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINE_STRIP);
            glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
            glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
            glEnd();
            glDisable(GL_LINE_STIPPLE);

            glPopAttrib();
        }

        if(!node->IsLeaf()) {
            int axis = node->SplitAxis();
            double plane = node->SplitPos();

            bbox = bb;
            bbox.min[axis] = plane;
            bbox.max[axis] = plane;
            if(mApp->aabbVisu.boxExpansion)
                bbox.Expand((mApp->aabbVisu.reverseExpansion ? -0.002 : 0.002) * delta[axis], axis);

            Vector3d a = bbox.min;
            Vector3d b = bbox.min;
            Vector3d c = bbox.max;
            Vector3d d = bbox.max;

            const int axisSwap = (axis + 2) % 3;
            b[axisSwap] = bbox.max[axisSwap];
            d[axisSwap] = bbox.min[axisSwap];

            // begin transparent sides
            if(selection && mApp->aabbVisu.selectedNode == currentNodeIndex) {
                glColor4f(color[0], color[1], color[2], std::max(color[3] * mApp->aabbVisu.alpha * 1.2f, 0.45f));
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else{
                glColor4f(color[0], color[1], color[2], color[3] * mApp->aabbVisu.alpha);
            }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBegin(GL_POLYGON);
            glVertex3d(a.x, a.y, a.z);
            glVertex3d(b.x, b.y, b.z);
            glVertex3d(c.x, c.y, c.z);
            glVertex3d(d.x, d.y, d.z);
            glEnd();
            // end transparent sides
        }

        if (mApp->antiAliasing) {
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
        }
        glDepthMask(GL_TRUE);
    }

    if(level < max_level && !node->IsLeaf()) {
        int axis = node->SplitAxis();
        double plane = node->SplitPos();

        auto bbl = bb.Split(axis, plane, true);
        auto bbr = bb.Split(axis, plane, false);
        if(mApp->aabbVisu.showBranchSide[0]) DrawAABBPlane(lnode, bbl, currentNodeIndex + 1, level + 1, selection);
        if(mApp->aabbVisu.showBranchSide[1]) DrawAABBPlane(lnode, bbr, node->AboveChild(), level + 1, selection);
    }
}
*/


void Geometry::DrawAABBNode(const BVHAccel &bvh) {
    if (!bvh.nodes || bvh.nodes[0].secondChildOffset == 0)
        return;

    int currentNodeIndex = 0;
    const LinearBVHNode *node = &bvh.nodes[currentNodeIndex];

    DrawAABBNode(node, currentNodeIndex, 0, false);
    if(mApp->aabbVisu.selectedNode > -1)
        DrawAABBNode(node, currentNodeIndex, 0, true);
}

void Geometry::DrawAABBNode(const LinearBVHNode *lnode, int currentNodeIndex, int level, bool selection) {

    const LinearBVHNode *node = &lnode[currentNodeIndex];

    const int max_level = (mApp->aabbVisu.showLevelAABB[1] == -1) ? 64 : mApp->aabbVisu.showLevelAABB[1];
    if(node->nPrimitives > 0 && !(mApp->aabbVisu.showAABBLeaves || mApp->aabbVisu.travStep))
        return;

    if((!mApp->aabbVisu.travStep || (mApp->aabbVisu.travStep && node->nPrimitives > 0)) && ((!selection || (selection && mApp->aabbVisu.selectedNode == currentNodeIndex)) && (mApp->aabbVisu.showLevelAABB[0] == -1 || ((mApp->aabbVisu.showLevelAABB[0] <= level) && (level <= mApp->aabbVisu.showLevelAABB[1]))))) {
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        if (mApp->antiAliasing) {
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
        }

        float rate = 0.0f;
        std::array<float,4> color{0.0f};
        if(mApp->aabbVisu.travStep && node->nPrimitives > 0){

        }
        else if(mApp->aabbVisu.showStats){
            //auto& stats = mApp->worker.model->kdtree.front().ints[currentNodeIndex];
            //rate = (stats.nbChecks > 0) ? (float) stats.nbIntersects / (float) stats.nbChecks : 0.0f;
            rate = (!mApp->aabbVisu.rateVector->empty()) ? mApp->aabbVisu.rateVector->at(currentNodeIndex) : 0.0f;
            if(rate >= mApp->aabbVisu.trimByProb[0] && rate <= mApp->aabbVisu.trimByProb[1]) {
                color = (mApp->aabbVisu.colorMap.empty()) ? applyGLColor4f(rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]) : applyGLColor4f(mApp->aabbVisu.colorMap, rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]);
            }
        }

        if(selection && mApp->aabbVisu.selectedNode == currentNodeIndex) {
            color = {0.9f ,
                     0.9f ,
                     0.2f ,
                     0.5f};
        }
        else if(mApp->aabbVisu.showStats){

        }
        else if(mApp->aabbVisu.sameColor){
            color = {0.6f ,
                     0.6f ,
                     std::max(0.7f, (1.0f / (1.0f + level * 0.5f))) ,
                     std::min(1.0f, (level * 0.02f) + mApp->aabbVisu.alpha)};
        }
        else {
            int minLevel=0, maxLevel=30;
            if(mApp->aabbVisu.showLevelAABB[0]>-1){
                minLevel = mApp->aabbVisu.showLevelAABB[0];
            }
            if(mApp->aabbVisu.showLevelAABB[1]>-1){
                maxLevel = mApp->aabbVisu.showLevelAABB[1];
            }
            color = (mApp->aabbVisu.colorMap.empty()) ? applyGLColor4f(level, minLevel, maxLevel) : applyGLColor4f(mApp->aabbVisu.colorMap, level, minLevel, maxLevel);
        }

        auto bbox = node->bounds;
        auto delta = this->bb.max - this->bb.min;
        if(mApp->aabbVisu.boxExpansion)
            bbox.Expand((mApp->aabbVisu.reverseExpansion ? -0.0002 : 0.0002)*delta*level);


        // begin transparent sides
        glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();


        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();
        // end transparent sides

        // corner points
        glPointSize(std::max(1.0f, 15.0f - level * 3.0f));
        glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 1.2f);

        glBegin(GL_POINTS);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();

        glPushAttrib(GL_LINE_BIT);

        glLineWidth(std::max(1.0f, 5.0f - level * 0.5f));

        // edges
        if(selection && mApp->aabbVisu.selectedNode == currentNodeIndex) {
            // Overwrite other boxes
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ZERO);
        }
        glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 4.0f);

        glBegin(GL_LINE_LOOP);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();

        glLineStipple(1, 0x0101);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_STRIP);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        glDisable(GL_LINE_STIPPLE);

        glPopAttrib();
        //glLineWidth(1.0f);

        if (mApp->antiAliasing) {
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
        }
        glDepthMask(GL_TRUE);
    }

    if(level < max_level && node->nPrimitives <= 0) {
        if(mApp->aabbVisu.showBranchSide[0]) DrawAABBNode(lnode, currentNodeIndex + 1, level + 1, selection);
        if(mApp->aabbVisu.showBranchSide[1]) DrawAABBNode(lnode, node->secondChildOffset, level + 1, selection);
    }
}


void Geometry::DrawAABBNode(AABBNODE* node, int level) {
    const int max_level = (mApp->aabbVisu.showLevelAABB[1] == -1) ? 64 : mApp->aabbVisu.showLevelAABB[1];
    if(node->facets.size() <= 1 && !mApp->aabbVisu.showAABBLeaves) return;

    if(mApp->aabbVisu.showLevelAABB[0] == -1 || ((mApp->aabbVisu.showLevelAABB[0] <= level) && (level <= mApp->aabbVisu.showLevelAABB[1]))) {
        auto bbox = node->bb;
        auto delta = this->bb.max - this->bb.min;
        if(mApp->aabbVisu.boxExpansion)
            bbox.Expand((mApp->aabbVisu.reverseExpansion ? -0.0002 : 0.0002)*delta*level);
        /*glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);*/
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        if (mApp->antiAliasing) {
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
        }


        // begin transparent sides
        glColor4f(
                (level % 3 == 0) ? (1.0f / (1.0f + ((int)(level / 3.0f)) * 0.1f)) : 0.2f,
                (level % 3 == 1) ? (1.0f / (1.0f + ((int)(level / 3.0f)) * 0.1f)) : 0.2f,
                (level % 3 == 2) ? (1.0f / (1.0f + ((int)(level / 3.0f)) * 0.1f)) : 0.2f,
                std::min(1.0f, ((int)(level / 3.0f) * 0.02f) + mApp->aabbVisu.alpha));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();


        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();
        // end transparent sides

        glPointSize(std::max(1.0f, 15.0f - level * 3.0f));
        glColor4f(
                (level % 3 == 0) ? (1.0f / (1.0f + level * 0.5)) : 0.1f,
                (level % 3 == 1) ? (1.0f / (1.0f + (level - 1) * 0.5)) : 0.1f,
                (level % 3 == 2) ? (1.0f / (1.0f + (level - 2) * 0.5)) : 0.1f,
                std::min(1.0f, (level * 0.02f) + mApp->aabbVisu.alpha));

        glBegin(GL_POINTS);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();

        glColor4f(
                (level % 3 == 0) ? (1.0f / (1.0f + level * 0.5)) : 0.1f,
                (level % 3 == 1) ? (1.0f / (1.0f + (level - 1) * 0.5)) : 0.1f,
                (level % 3 == 2) ? (1.0f / (1.0f + (level - 2) * 0.5)) : 0.1f,
                std::min(1.0f, (level * 0.05f) + 0.4f));

        glLineWidth(3.0f);

        glPushAttrib(GL_LINE_BIT);
        glLineWidth(std::max(1.0f, 10.0f - level * 3.0f));
        glColor4f(
                (level % 3 == 0) ? (1.0f / (1.0f + (level) * 0.5)) : 0.2f,
                (level % 3 == 1) ? (1.0f / (1.0f + (level - 1) * 0.5)) : 0.2f,
                (level % 3 == 2) ? (1.0f / (1.0f + (level - 2) * 0.5)) : 0.2f,
                std::min(1.0f, (level * 0.5f) + 0.1f));

        glBegin(GL_LINE_LOOP);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();

        glLineStipple(1, 0x0101);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_STRIP);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        glDisable(GL_LINE_STIPPLE);

        glPopAttrib();
        //glLineWidth(1.0f);

        if (mApp->antiAliasing) {
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
        }
        glDepthMask(GL_TRUE);
    }

    if(level < max_level) {
        if(node->left && mApp->aabbVisu.showBranchSide[0]) DrawAABBNode(node->left, level+1);
        if(node->right && mApp->aabbVisu.showBranchSide[1]) DrawAABBNode(node->right, level+1);
    }

}

void Geometry::DrawHeatmap(const SubprocessFacet& facet) {


    if(facet.nbTests == 0 || facet.nbTraversalSteps == 0)
        return;

    if(1) {

        //float rate = (facet.nbTraversalSteps > 0) ? (double)facet.nbTraversalSteps / (double)facet.nbTests : 0.0;
        float rate = (!mApp->aabbVisu.rateVector->empty() ? mApp->aabbVisu.rateVector->at(facet.globalId) : 0.0f);
        //std::array<float,4> color{0.0f};
        std::array<float,4> color = (mApp->aabbVisu.colorMap.empty()) ? applyGLColor4f(rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]) : applyGLColor4f(mApp->aabbVisu.colorMap, rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]);
        /*if(rate >= mApp->aabbVisu.trimByProb[0] && rate <= mApp->aabbVisu.trimByProb[1]) {
                    color = (mApp->aabbVisu.colorMap.empty()) ? applyGLColor4f(rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]) : applyGLColor4f(mApp->aabbVisu.colorMap, rate, mApp->aabbVisu.trimByProb[0], mApp->aabbVisu.trimByProb[1]);
                }*/

        auto bbox = facet.sh.bb;
        auto delta = this->bb.max - this->bb.min;
        if(mApp->aabbVisu.boxExpansion)
            bbox.Expand((mApp->aabbVisu.reverseExpansion ? -0.0002 : 0.0002)*delta);
        /*glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);*/
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        if (mApp->antiAliasing) {
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
        }
        /* if(selection && mApp->aabbVisu.selectedNode == currentNodeIndex) {
             glDisable(GL_DEPTH_TEST);
             glColor4f(
                     0.9f ,
                     0.2f ,
                     0.2f ,
                     0.5f);
         }*/

        // begin transparent sides
        glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();


        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();
        // end transparent sides

        // corner points
        glPointSize(std::max(1.0f, 12.0f));
        glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 1.2f);


        glBegin(GL_POINTS);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();

        glPushAttrib(GL_LINE_BIT);

        glLineWidth(std::max(1.0f, 7.0f));

        // edges
        glColor4f(color[0],color[1],color[2],color[3] * mApp->aabbVisu.alpha * 1.1f);

        glBegin(GL_LINE_LOOP);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();

        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.min.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.min.y, bbox.max.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.max.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bbox.min.x, bbox.max.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.min.z);
        glEnd();

        glLineStipple(1, 0x0101);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_STRIP);
        glVertex3d(bbox.min.x, bbox.min.y, bbox.min.z);
        glVertex3d(bbox.max.x, bbox.max.y, bbox.max.z);
        glEnd();
        glDisable(GL_LINE_STIPPLE);

        glPopAttrib();
        //glLineWidth(1.0f);

        if (mApp->antiAliasing) {
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
        }
        glDepthMask(GL_TRUE);
    }
}

void Geometry::DrawAABB() {
    if(mApp->aabbVisu.travStep && !mApp->worker.model->facets.empty()){
        for (auto& f : mApp->worker.model->facets){
            DrawHeatmap(*f);
        }
    }
    else if(!mApp->worker.model->structures.empty()) {
        if(mApp->aabbVisu.oldBVH) {
            std::vector<std::vector<SubprocessFacet *>> facetPointers;
            facetPointers.resize(mApp->worker.model->sh.nbSuper);
            for (size_t s = 0; s < mApp->worker.model->sh.nbSuper; ++s) {
                for (auto &sFac : mApp->worker.model->facets) {
                    // TODO: Build structures
                    if (sFac->sh.superIdx == -1) { //Facet in all structures
                        for (auto &fp_vec : facetPointers) {
                            fp_vec.push_back(sFac.get());
                        }
                    } else {
                        facetPointers[sFac->sh.superIdx].push_back(sFac.get()); //Assign to structure
                    }
                }
            }

            // Build all AABBTrees
            size_t maxDepth = 0;
            std::vector<AABBNODE *> tree(mApp->worker.model->sh.nbSuper);
            for (size_t s = 0; s < mApp->worker.model->sh.nbSuper; ++s) {
                tree[s] = BuildAABBTree(facetPointers[s], 0, maxDepth);
            }

            int nStructs = (mApp->aabbVisu.drawAllStructs ? mApp->worker.model->sh.nbSuper : (int)1);
            for (int s = 0; s < nStructs; ++s)
                if (tree[s]) {
                    DrawAABBNode(tree[s], 0);
                }
            for (int s = 0; s < nStructs; ++s)
                delete tree[s];
        }
        else {

            std::vector<std::shared_ptr<RTAccel>>& accel = mApp->worker.model->accel;
            /*if(mApp->worker.model->accel.empty()) {
                mApp->worker.model->BuildAccelStructure(
                        &mApp->worker.globState,
                        mApp->worker.model->wp.accel_type,
                        mApp->aabbVisu.splitTechnique,
                        mApp->worker.model->wp.bvhMaxPrimsInNode
                        );
            }*/
            //accel = mApp->worker.model->accel;

            if(!accel.empty() && mApp->worker.model->initialized) {
                size_t nStructs = (mApp->aabbVisu.drawAllStructs ? mApp->worker.model->sh.nbSuper : 1);
                for (int s = 0; s < nStructs; ++s){
                    if(dynamic_cast<BVHAccel*>(accel.at(s).get())){
                        DrawAABBNode(*dynamic_cast<BVHAccel*>(accel.at(s).get()));
                    }
                    else if(dynamic_cast<KdTreeAccel*>(accel.at(s).get())){
                        DrawAABBNode(*dynamic_cast<KdTreeAccel*>(accel.at(s).get()));
                    }

                }
            }
        }
    }
}

void Geometry::DrawPolys() {

	std::vector<size_t> f3; f3.reserve(sh.nbFacet);
	std::vector<size_t> f4; f4.reserve(sh.nbFacet);
	std::vector<size_t> fp; fp.reserve(sh.nbFacet);

	// Group TRI,QUAD and POLY
	for (size_t i = 0; i < sh.nbFacet; i++) {
		size_t nb = facets[i]->sh.nbIndex;
		if (facets[i]->volumeVisible
		//&& !facets[i]->selected // don't draw part of the volume that is selected
		) {
			if (nb == 3) {
				f3.push_back(i);
			}
			else if (nb == 4) {
				f4.push_back(i);
			}
			else {
				fp.push_back(i);
			}
		}
	}

	// Draw
	glBegin(GL_TRIANGLES);

	// Triangle
	for (const auto& i : f3)
		FillFacet(facets[i], false);

	// Triangulate polygon
	for (const auto& i : fp)
		Triangulate(facets[i], false);

	glEnd();

	// Quads
	glBegin(GL_QUADS);
	for (const auto& i : f4)
		FillFacet(facets[i], false);
	glEnd();
}

// returns 1 if lhs is greater, -1 otherwise
float Geometry::getMaxDistToCamera(InterfaceFacet* f){

    float rx, ry, rz, rw;

    GLfloat mProj[16];
    GLfloat mView[16];
    GLVIEWPORT g;

    glGetFloatv(GL_PROJECTION_MATRIX, mProj);
    glGetFloatv(GL_MODELVIEW_MATRIX, mView);
    glGetIntegerv(GL_VIEWPORT, (GLint *)&g);

    GLMatrix proj; proj.LoadGL(mProj);
    GLMatrix view; view.LoadGL(mView);
    GLMatrix m; m.Multiply(&proj, &view);
    float distToCamera = -99999.99f;

    for(int i=0;i<f->sh.nbIndex;++i){
        size_t idx = f->indices[i];
        m.TransfomVec((float)vertices3[idx].x, (float)vertices3[idx].y, (float)vertices3[idx].z, 1.0f,
                      &rx, &ry, &rz, &rw);
        distToCamera = std::max(rz,distToCamera);
    }
    return distToCamera;
}

// returns 1 if lhs is greater, -1 otherwise
int Geometry::compareFacetDepth(InterfaceFacet* lhs, InterfaceFacet* rhs){

    if(getMaxDistToCamera(lhs) > getMaxDistToCamera(rhs)){
        return 1;
    }
    else return 0;

}
void Geometry::DrawTransparentPolys(const std::vector<size_t> &selectedFacets) {

    //std::vector<size_t> f3; f3.reserve(sh.nbFacet);
    //std::vector<size_t> f4; f4.reserve(sh.nbFacet);
    //std::vector<size_t> fp; fp.reserve(sh.nbFacet);


    //---Draw transparent selected polygons
    // Group TRI,QUAD and POLY
    /*for (int i = 0; i < selectedFacets.size(); ++i) {
        const size_t nb = facets[i]->sh.nbIndex;
        if (nb == 3) {
            f3.push_back(selectedFacets[i]);
        }
        else if (nb==4){
            f4.push_back(selectedFacets[i]);
        }
        else{
            fp.push_back(selectedFacets[i]);
        }
    }*/
    /*std::list<size_t> sortedFacets;
    for (int i = 0; i < selectedFacets.size(); ++i) {
        *//*bool isBiggest = true;
        for (std::list<size_t>::iterator it=sortedFacets.begin(); it!=sortedFacets.end(); ++it){
            if(compareFacetDepth(facets[selectedFacets[i]],facets[*it])){
                continue;
            }
            else{
                isBiggest = false;
                sortedFacets.insert(it,selectedFacets[i]);
            }
        }
        if(isBiggest)*//*
            sortedFacets.push_back(selectedFacets[i]);
    }*/

    const auto colorHighlighting = mApp->worker.GetGeometry()->GetPlottedFacets(); // For colors
    // Draw
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	struct ArrowToDraw {
		Vector3d startPoint, endPoint, parallel;
		std::array<float,4> color; //to pass components to glColor4f
	};
	std::vector<ArrowToDraw> arrowsToDraw;

    glBegin(GL_TRIANGLES);
    for (const auto& sel : selectedFacets) {
        if(!colorHighlighting.empty()) {
            auto it = colorHighlighting.find(sel);
            // Check if element exists in map or not
			auto profileMode = facets[sel]->sh.profileType;
			ArrowToDraw arrow;
			if (it != colorHighlighting.end()) {
				float r = static_cast<float>(it->second.r) / 255.0f;
				float g = static_cast<float>(it->second.g) / 255.0f;
				float b = static_cast<float>(it->second.b) / 255.0f;
				glColor4f(r, g, b, 0.5f);
				arrow.color = {r, g, b, 0.3f};
            } else {
                glColor4f(0.937f,0.957f,1.0f, 0.08f);    //metro light blue
				arrow.color = {0.937f,0.957f,1.0f, 0.08f};
            }
			if (profileMode == PROFILE_U || profileMode == PROFILE_V) {
				Vector3d& center = facets[sel]->sh.center;
				Vector3d& dir = profileMode == PROFILE_U ? facets[sel]->sh.U : facets[sel]->sh.V;
				arrow.startPoint = center - .5 * dir;
				arrow.endPoint = center + .5* dir;
				arrow.parallel = profileMode == PROFILE_U ? facets[sel]->sh.nV : facets[sel]->sh.nU;
				arrowsToDraw.push_back(arrow);
			}
        }
        else{
            glColor4f(0.933f, 0.067f, 0.067f, 0.15f);    //metro red
        }
        size_t nb = facets[sel]->sh.nbIndex;
        if (nb == 3) {
            FillFacet(facets[sel], false);
        }
        else {
            Triangulate(facets[sel], false);
        }
    }
    glEnd();

	AxisAlignedBoundingBox bb = GetBB();
	double arrowLength = 30.0 / Max((bb.max.x - bb.min.x), (bb.max.y - bb.min.y));

	glPushAttrib(GL_ENABLE_BIT);
	glLineStipple(2, 0xAAAA);
	glEnable(GL_LINE_STIPPLE);
	for (const auto& arr : arrowsToDraw) {
		glColor4f(arr.color[0], arr.color[1], arr.color[2], arr.color[3]);
		GLToolkit::DrawVector(arr.startPoint, arr.endPoint, arr.parallel);
	}
	glPopAttrib();
    //---end transparent
}

void Geometry::SetCullMode(int mode) {

	switch (mode) {
	case 1: // SHOW_FRONT
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case 2: // SHOW_BACK
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	default: //SHOW_FRONTANDBACK
		glDisable(GL_CULL_FACE);
	}

}

void Geometry::ClearFacetTextures()
{
	GLProgress *prg = new GLProgress("Clearing texture", "Frame update");
	prg->SetBounds(5, 28, 300, 90);
	int startTime = SDL_GetTicks();
	for (int i = 0; i<sh.nbFacet; i++) {
		if (!prg->IsVisible() && ((SDL_GetTicks() - startTime) > 500)) {
			prg->SetVisible(true);
		}
		prg->SetProgress((double)i / (double)sh.nbFacet);
		DELETE_TEX(facets[i]->glTex);
		glGenTextures(1, &facets[i]->glTex);
	}
	prg->SetVisible(false);
	SAFE_DELETE(prg);
}

void Geometry::RenderArrow(GLfloat *matView, float dx, float dy, float dz, float px, float py, float pz, float d) {

	if (!arrowList) BuildShapeList();

	// Compute transformation matrix for the arrow
	GLMatrix mView;
	GLMatrix aView;
	GLMatrix mScale;
	GLMatrix mRot;
	GLMatrix mT;
	float v1x, v1y, v1z;

	mView.LoadGL(matView);

	// Direction
	float n = sqrtf(dx*dx + dy*dy + dz*dz);
	if (IsZero(n)) {

		// Isotropic (render a sphere)
		mScale._11 = (d / 4.0f);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);

		mT.Translate(px, py, pz);

		aView.Multiply(&mView);
		aView.Multiply(&mT);
		aView.Multiply(&mScale);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(aView.GetGL());
		glCallList(sphereList);
		return;

	}

	dx /= n;
	dy /= n;
	dz /= n;
	mRot._11 = dx;
	mRot._21 = dy;
	mRot._31 = dz;

	// A point belonging to the plane
	// normal to the direction vector
	if (!IsZero(dx)) {
		v1x = -dz / dx;
		v1y = 0.0;
		v1z = 1.0;
	}
	else if (!IsZero(dy)) {
		v1x = 0.0;
		v1y = -dz / dy;
		v1z = 1.0;
	}
	else if (!IsZero(dz)) {
		// normal to z
		v1x = 1.0;
		v1y = 0.0;
		v1z = 0.0;
	}
	else {

		// Null vector -> isotropic
	}

	float n1 = sqrtf(v1x*v1x + v1y*v1y + v1z*v1z);
	v1x /= n1;
	v1y /= n1;
	v1z /= n1;
	mRot._12 = v1x;
	mRot._22 = v1y;
	mRot._32 = v1z;

	// Cross product
	mRot._13 = (dy)*(v1z)-(dz)*(v1y);
	mRot._23 = (dz)*(v1x)-(dx)*(v1z);
	mRot._33 = (dx)*(v1y)-(dy)*(v1x);

	// Scale
	if (!autoNorme) {
		mScale._11 = (n*d*normeRatio);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);
	}
	else {

		// Show only direction
		mScale._11 = (d / 1.1f);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);
	}

	mT.Translate(px, py, pz);

	aView.Multiply(&mView);
	aView.Multiply(&mT);
	aView.Multiply(&mRot);
	aView.Multiply(&mScale);
	if (!centerNorme) {
		mT.Translate(0.5f, 0.0, 0.0);
		aView.Multiply(&mT);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(aView.GetGL());
	glCallList(arrowList);

}

// Triangulation stuff

int Geometry::FindEar(const GLAppPolygon& p) {

	int i = 0;
	bool earFound = false;
	while (i < p.pts.size() && !earFound) {
		if (IsConvex(p, i))
			earFound = !ContainsConcave(p, i - 1, i, i + 1);
		if (!earFound) i++;
	}

	// REM: Theoritically, it should always find an ear (2-Ears theorem).
	// However on degenerated geometry (flat poly) it may not find one.
	// Returns first point in case of failure.
	if (earFound)
		return i;
	else
		return 0;

}

void Geometry::AddTextureCoord(InterfaceFacet *f, const Vector2d *p) {

	// Add texture coord with a 1 texel border (for bilinear filtering)
	double uStep = 1.0 / (double)f->texDimW;
	double vStep = 1.0 / (double)f->texDimH;

#if 1
	double fu = f->sh.texWidth_precise * uStep;
	double fv = f->sh.texHeight_precise * vStep;
	glTexCoord2f((float)(uStep + p->u*fu), (float)(vStep + p->v*fv));
#else
	// Show border (debugging purpose)
	double fu = (f->sh.texWidth_precise + 2.0) * uStep;
	double fv = (f->sh.texHeight_precise + 2.0) * vStep;
	glTexCoord2f((float)(p->u*fu), (float)(p->v*fv));
#endif

}

void Geometry::FillFacet(InterfaceFacet *f, bool addTextureCoord) {
	//Commented out sections: theoretically in a right-handed system the vertex order is inverse
	//However we'll solve it simpler by inverting the geometry viewer Front/back culling mode setting

	glNormal3d(-f->sh.N.x, -f->sh.N.y, -f->sh.N.z);
	/*size_t nbDrawn = 0;
	size_t i;
	if (mApp->leftHandedView) {
			i = 0;
			glNormal3d(-f->wp.N.x, -f->wp.N.y, -f->wp.N.z);
	}
	else {
			i = f->wp.nbIndex-1;
			glNormal3d(f->wp.N.x, f->wp.N.y, f->wp.N.z);
	}
	for (; nbDrawn < f->wp.nbIndex; nbDrawn++) {*/
	for (size_t i=0;i<f->sh.nbIndex;i++) {
		size_t idx = f->indices[i];
		if (addTextureCoord) AddTextureCoord(f, &(f->vertices2[i]));
		glVertex3d(vertices3[idx].x, vertices3[idx].y, vertices3[idx].z);
		/*if (mApp->leftHandedView) {
			i++;
		}
		else {
			i--;
		}*/
	}
}

void Geometry::DrawEar(InterfaceFacet *f, const GLAppPolygon& p, int ear, bool addTextureCoord) {

	//Commented out sections: theoretically in a right-handed system the vertex order is inverse
	//However we'll solve it simpler by inverting the geometry viewer Front/back culling mode setting

	Vector3d  p3D;
	const Vector2d* p1;
	const Vector2d* p2;
	const Vector2d* p3;

	// Follow orientation
	/*double handedness = mApp->leftHandedView ? 1.0 : -1.0;*/
	
	//if (/*handedness * */ p.sign > 0) {
	//	p1 = &(p.pts[Previous(ear, p.pts.size())]);
	//	p2 = &(p.pts[Next(ear, p.pts.size())]);
	//	p3 = &(p.pts[IDX(ear, p.pts.size())]);
	//}
	//else {
		p1 = &(p.pts[Previous(ear, p.pts.size())]);
		p2 = &(p.pts[IDX(ear, p.pts.size())]);
		p3 = &(p.pts[Next(ear, p.pts.size())]);
	//}

	glNormal3d(-f->sh.N.x, -f->sh.N.y, -f->sh.N.z);
	if (addTextureCoord) AddTextureCoord(f, p1);
	f->glVertex2u(p1->u, p1->v);

	//glNormal3d(-f->wp.N.x, -f->wp.N.y, -f->wp.N.z);
	if (addTextureCoord) AddTextureCoord(f, p2);
	f->glVertex2u(p2->u, p2->v);

	//glNormal3d(-f->wp.N.x, -f->wp.N.y, -f->wp.N.z);
	if (addTextureCoord) AddTextureCoord(f, p3);
	f->glVertex2u(p3->u, p3->v);

}

void Geometry::Triangulate(InterfaceFacet *f, bool addTextureCoord) {

	// Triangulate a facet (rendering purpose)
	// The facet must have at least 3 points
	// Use the very simple "Two-Ears" theorem. It computes in O(n^2).

	if (f->nonSimple) {
		// Not a simple polygon
		// Abort triangulation
		return;
	}

	// Build a Polygon
	GLAppPolygon p;
	p.pts = f->vertices2;
	//p.sign = f->sign;
	
	// Perform triangulation
	while (p.pts.size() > 3) {
		int e = FindEar(p);
		DrawEar(f, p, e, addTextureCoord);
		// Remove the ear
		p.pts.erase(p.pts.begin() + e);
	}

	// Draw the last ear
	DrawEar(f, p, 0, addTextureCoord);

}

void Geometry::Render(GLfloat *matView, bool renderVolume, bool renderTexture, int showMode, bool filter, bool showHidden, bool showMesh, bool showDir) {

	if (!isLoaded) return;

	// Render the geometry
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	// Render Volume
	if (renderVolume) {
		glPolygonOffset(1.0f, 4.0f);
		SetCullMode(showMode);
		GLfloat global_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHTING);
		GLToolkit::SetMaterial(&fillMaterial);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glCallList(polyList);
		glDisable(GL_POLYGON_OFFSET_FILL);
		GLToolkit::SetMaterial(&whiteMaterial);
		glDisable(GL_LIGHTING);
	}
	else {

		// Default material
		GLToolkit::SetMaterial(&whiteMaterial);

		// Draw lines
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);

		float color = (mApp->whiteBg) ? 0.0f : 1.0f; //whitebg here
		if (viewStruct == -1) {
			glColor4f(color, color, color, 0.5f);
			if (mApp->antiAliasing) {
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			}
			glPolygonOffset(1.0f, 5.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			for (int i = 0;i < sh.nbSuper;i++)
				glCallList(lineList[i]);
			glDisable(GL_POLYGON_OFFSET_LINE);
			glDisable(GL_BLEND);
			glDisable(GL_LINE_SMOOTH);
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		else {

			// Draw non selectable facet in dark grey
			glColor3f(0.2f, 0.2f, 0.2f);
			if (mApp->antiAliasing) {
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			}
			for (int i = 0;i < sh.nbSuper;i++)
				if (viewStruct != i)
					glCallList(lineList[i]);
			// Selectable in white
			glColor3f(color, color, color);
			glPolygonOffset(1.0f, 5.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glCallList(lineList[viewStruct]);
			glDisable(GL_POLYGON_OFFSET_LINE);
			if (mApp->antiAliasing) {
				glDisable(GL_BLEND);
				glDisable(GL_LINE_SMOOTH);
			}
		}
	}

	// Paint texture
	if (renderTexture) {
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 3.0f);
		for (size_t i = 0;i < sh.nbFacet && renderTexture;i++) {
			InterfaceFacet *f = facets[i];
			bool paintRegularTexture = f->sh.isTextured && f->textureVisible && (f->sh.countAbs || f->sh.countRefl || f->sh.countTrans);
#if defined(MOLFLOW)
			paintRegularTexture = paintRegularTexture || (f->sh.isTextured && f->textureVisible && (f->sh.countACD || f->sh.countDes));
#endif
			if (paintRegularTexture) {
				if (f->sh.is2sided)   glDisable(GL_CULL_FACE);
				else                   SetCullMode(showMode);
				glBindTexture(GL_TEXTURE_2D, f->glTex);
				if (filter) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
				else {

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}
				
				glCallList(f->glList);
			}
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	}

	// Paint mesh
	if (showMesh) {
		glColor4f(0.7f, 0.7f, 0.7f, 0.3f);
		if (mApp->antiAliasing) {
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < sh.nbFacet;i++) {

			InterfaceFacet *f = facets[i];
			if (!f->cellPropertiesIds.empty()  && f->textureVisible) {
				if (!f->glElem) f->BuildMeshGLList();

				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(1.0f, 2.0f);
				glCallList(f->glElem);
				glDisable(GL_POLYGON_OFFSET_LINE);
			}
		}
		if (mApp->antiAliasing) {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
		}
	}

	// Paint direction fields
	if (showDir) {

		GLToolkit::SetMaterial(&arrowMaterial);
		for (int i = 0;i < sh.nbFacet;i++) {
			InterfaceFacet *f = facets[i];
			if (f->sh.countDirection && f->dirCache) {
				double iw = 1.0 / (double)f->sh.texWidth_precise;
				double ih = 1.0 / (double)f->sh.texHeight_precise;
				double rw = f->sh.U.Norme() * iw;
				for (int x = 0;x < f->sh.texWidth;x++) {
					for (int y = 0;y < f->sh.texHeight;y++) {
						size_t add = x + y*f->sh.texWidth;
						if (f->GetMeshArea(add) > 0.0) {
							double uC = ((double)x + 0.5) * iw;
							double vC = ((double)y + 0.5) * ih;
							float xc = (float)(f->sh.O.x + f->sh.U.x*uC + f->sh.V.x*vC);
							float yc = (float)(f->sh.O.y + f->sh.U.y*uC + f->sh.V.y*vC);
							float zc = (float)(f->sh.O.z + f->sh.U.z*uC + f->sh.V.z*vC);

							RenderArrow(matView,
								(float)f->dirCache[add].dir.x,
								(float)f->dirCache[add].dir.y,
								(float)f->dirCache[add].dir.z,
								xc, yc, zc, (float)rw); // dircache already normalized
						}
					}
				}
			}
		}

		// Restore default matrix
		glLoadMatrixf(matView);
	}

    if(mApp->aabbVisu.renderAABB) // always draw for now
        glCallList(aabbList);
    //DrawAABB();

	// Paint non-planar and selected facets
	//if (GetNbSelectedFacets()>0) {
		if (mApp->antiAliasing) {
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
		}
		glBlendFunc(GL_ONE, GL_ZERO);
		if (mApp->highlightNonplanarFacets) {
			glColor3f(1.0f, 0.0f, 1.0f);    //purple
			glCallList(nonPlanarList);
		}
		glColor3f(1.0f, 0.0f, 0.0f);    //red
		if (showHidden) {
			glDisable(GL_DEPTH_TEST);
			glCallList(selectList3);
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glCallList(selectList3);
		}
		if (mApp->antiAliasing) {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
		}

	//}

	// Paint selected cell on mesh
	for (int i = 0; i < sh.nbFacet;i++) {
		InterfaceFacet *f = facets[i];
		f->RenderSelectedElem();
	}
}

void Geometry::RenderSemiTransparent(GLfloat *matView, bool renderVolume, bool renderTexture, int showMode, bool filter, bool showHidden, bool showMesh, bool showDir) {
    if (mApp->antiAliasing) {
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
    }

    glCallList(selectHighlightList);

    if (mApp->antiAliasing) {
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_BLEND);
    }
}

void Geometry::DeleteGLLists(bool deletePoly, bool deleteLine) {
	if (deleteLine) {
		for (int i = 0; i < sh.nbSuper; i++)
			DELETE_LIST(lineList[i]);
	}
	if (deletePoly) DELETE_LIST(polyList);
    DELETE_LIST(aabbList);
    DELETE_LIST(selectList);
	DELETE_LIST(selectList2);
    DELETE_LIST(selectList3);
    DELETE_LIST(selectHighlightList);
}

std::vector<bool> Geometry::GetVertexBelongsToSelectedFacet() {
	std::vector<bool> result(sh.nbVertex, false);
	std::vector<size_t> selFacetIds = GetSelectedFacets();
	for (auto& facetId : selFacetIds) {
		InterfaceFacet* f = facets[facetId];
		for (size_t i = 0; i < f->sh.nbIndex; i++)
			result[f->indices[i]] = true;
	}
	return result;
}

void Geometry::BuildShapeList() {

	// Shapes used for direction field rendering

	// 3D arrow (direction field)
	int nbDiv = 10;
	double alpha = 2.0*PI / (double)nbDiv;

	arrowList = glGenLists(1);
	glNewList(arrowList, GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glBegin(GL_TRIANGLES);

	// Arrow
	for (int i = 0; i < nbDiv; i++) {

		double y1 = sin(alpha*(double)i);
		double z1 = cos(alpha*(double)i);
		double y2 = sin(alpha*(double)((i + 1) % nbDiv));
		double z2 = cos(alpha*(double)((i + 1) % nbDiv));

		glNormal3d(0.0, y1, z1);
		glVertex3d(-0.5, 0.5*y1, 0.5*z1);
		glNormal3d(1.0, 0.0, 0.0);
		glVertex3d(0.5, 0.0, 0.0);
		glNormal3d(0.0, y2, z2);
		glVertex3d(-0.5, 0.5*y2, 0.5*z2);

	}

	// Cap facets
	for (int i = 0; i < nbDiv; i++) {

		double y1 = sin(alpha*(double)i);
		double z1 = cos(alpha*(double)i);
		double y2 = sin(alpha*(double)((i + 1) % nbDiv));
		double z2 = cos(alpha*(double)((i + 1) % nbDiv));

		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.5*y1, 0.5*z1);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.5*y2, 0.5*z2);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.0, 0.0);

	}

	glEnd();
	glEndList();

	// Shpere list (isotropic case)
	int nbPhi = 16;
	int nbTetha = 7;
	double dphi = 2.0*PI / (double)(nbPhi);
	double dtetha = PI / (double)(nbTetha + 1);

	sphereList = glGenLists(1);
	glNewList(sphereList, GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glBegin(GL_TRIANGLES);

	for (int i = 0; i <= nbTetha; i++) {
		for (int j = 0; j < nbPhi; j++) {

			Vector3d v1, v2, v3, v4;

			v1.x = sin(dtetha*(double)i)*cos(dphi*(double)j);
			v1.y = sin(dtetha*(double)i)*sin(dphi*(double)j);
			v1.z = cos(dtetha*(double)i);

			v2.x = sin(dtetha*(double)(i + 1))*cos(dphi*(double)j);
			v2.y = sin(dtetha*(double)(i + 1))*sin(dphi*(double)j);
			v2.z = cos(dtetha*(double)(i + 1));

			v3.x = sin(dtetha*(double)(i + 1))*cos(dphi*(double)(j + 1));
			v3.y = sin(dtetha*(double)(i + 1))*sin(dphi*(double)(j + 1));
			v3.z = cos(dtetha*(double)(i + 1));

			v4.x = sin(dtetha*(double)i)*cos(dphi*(double)(j + 1));
			v4.y = sin(dtetha*(double)i)*sin(dphi*(double)(j + 1));
			v4.z = cos(dtetha*(double)i);

			if (i < nbTetha) {
				glNormal3d(v1.x, v1.y, v1.z);
				glVertex3d(v1.x, v1.y, v1.z);
				glNormal3d(v2.x, v2.y, v2.z);
				glVertex3d(v2.x, v2.y, v2.z);
				glNormal3d(v3.x, v3.y, v3.z);
				glVertex3d(v3.x, v3.y, v3.z);
			}

			if (i > 0) {
				glNormal3d(v1.x, v1.y, v1.z);
				glVertex3d(v1.x, v1.y, v1.z);
				glNormal3d(v3.x, v3.y, v3.z);
				glVertex3d(v3.x, v3.y, v3.z);
				glNormal3d(v4.x, v4.y, v4.z);
				glVertex3d(v4.x, v4.y, v4.z);
			}

		}
	}

	glEnd();
	glEndList();

}

void Geometry::BuildSelectList() {

	selectList = glGenLists(1);
	glNewList(selectList, GL_COMPILE);
	/*
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	if (antiAliasing){
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	//glBlendFunc(GL_ONE,GL_ZERO);
	}
	glLineWidth(2.0f);

	for(int i=0;i<wp.nbFacet;i++ ) {
	Facet *f = facets[i];
	if( f->selected ) {
	//DrawFacet(f,false);
	DrawFacet(f,1,1,1);
	nbSelected++;
	}
	}
	glLineWidth(1.0f);
	if (antiAliasing) {
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	}*/
	glEndList();

	// Second list for usage with POLYGON_OFFSET
	selectList2 = glGenLists(1);
	glNewList(selectList2, GL_COMPILE);
	/*
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	if (antiAliasing){
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}
	glLineWidth(2.0f);

	for(int i=0;i<wp.nbFacet;i++ ) {
	Facet *f = facets[i];
	if( f->selected )
	{
	//DrawFacet(f,true,false,true);
	DrawFacet(f,1,1,1);
	}
	}
	glLineWidth(1.0f);
	if (antiAliasing) {
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	}*/
	glEndList();

	// Third list with hidden (hole join) edge visible
	selectList3 = glGenLists(1);
	glNewList(selectList3, GL_COMPILE);

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	if (mApp->antiAliasing) {
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}
	glLineWidth(2.0f);

	const auto selectedFacets = GetSelectedFacets();
    const auto colorHighlighting = mApp->worker.GetGeometry()->GetPlottedFacets(); // For colors

    for (auto& sel : selectedFacets) {
		InterfaceFacet *f = facets[sel];
		//DrawFacet(f,false,true,true);
        if(!colorHighlighting.empty()){
            auto it = colorHighlighting.find(sel);
            // Check if element exists in map or not
            if (it != colorHighlighting.end()) {
                continue;
            } else {
                glLineWidth(1.5f);
                glColor3f(0.937f,0.957f,1.0f);    //metro light blue
            }
        }
        else{
            glColor3f(1.0f, 0.0f, 0.0f);    //red
        }
		DrawFacet(f, false, true, false); //Faster than true true true, without noticeable glitches
        glLineWidth(2.0f);
    }
    // give profiled selection priority for being rendered last
    if(!colorHighlighting.empty()){
        for (auto& sel : selectedFacets) {
            auto it = colorHighlighting.find(sel);
            // Check if element exists in map or not
            if (it != colorHighlighting.end()) {
                glLineWidth(3.0f);
                float r = static_cast<float>(it->second.r) / 255.0f;
                float g = static_cast<float>(it->second.g) / 255.0f;
                float b = static_cast<float>(it->second.b) / 255.0f;

                // incrase brightness and saturity
                modifyRGBColor(r, g, b, 1.2f, 1.2f);

                glColor3f(r, g, b);
                InterfaceFacet *f = facets[sel];
                DrawFacet(f, false, true, false); //Faster than true true true, without noticeable glitches
                glLineWidth(2.0f);
            }
        }
    }
	glLineWidth(1.0f);
	if (mApp->antiAliasing) {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}
	glEndList();

	if(mApp->highlightSelection){ //Above 500 selected facets rendering can be slow
        // Fourth list with transparent highlighting for selected facets
        selectHighlightList = glGenLists(1);
        glNewList(selectHighlightList, GL_COMPILE);
		if (GetNbSelectedFacets() < 500) {  //Above 500 selected facets rendering can be slow
			glDepthMask(GL_FALSE);
			/*glDisable(GL_CULL_FACE);
			glDepthFunc(GL_LEQUAL);*/
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			//glDisable(GL_BLEND);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			glEnable(GL_BLEND);
			//glBlendEquation(GL_MAX);
			//glEnable(GL_MULTISAMPLE);
			//glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

			DrawTransparentPolys(selectedFacets);

			//glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			//glDisable(GL_MULTISAMPLE);
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
        glEndList();
	}


}

void Geometry::BuildAABBList() {
    aabbList = glGenLists(1);
    glNewList(aabbList, GL_COMPILE);
    DrawAABB();
    glEndList();
}

void Geometry::BuildVolumeFacetList() {
    polyList = glGenLists(1);
    glNewList(polyList, GL_COMPILE);
    DrawPolys();
    glEndList();
}

void Geometry::BuildNonPlanarList() {

	
	nonPlanarList = glGenLists(1);
	glNewList(nonPlanarList, GL_COMPILE);

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	if (mApp->antiAliasing) {
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}
	glLineWidth(2.0f);

	auto nonPlanarFacetIds = GetNonPlanarFacetIds();
	hasNonPlanar = !nonPlanarFacetIds.empty();
	for (const auto& np : nonPlanarFacetIds) {
		InterfaceFacet *f = facets[np];
		//DrawFacet(f,false,true,true);
		DrawFacet(f, false, true, false); //Faster than true true true, without noticeable glitches
	}
	glLineWidth(1.0f);
	if (mApp->antiAliasing) {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}
	glEndList();

}


void Geometry::UpdateSelection() {

	//DeleteGLLists();
    DELETE_LIST(selectList);
    DELETE_LIST(selectList2);
    DELETE_LIST(selectList3);
    DELETE_LIST(selectHighlightList);
	BuildSelectList();

}

void Geometry::BuildGLList() {

	// Compile geometry for OpenGL
	for (int j = 0; j < sh.nbSuper; j++) {
		lineList[j] = glGenLists(1);
		glNewList(lineList[j], GL_COMPILE);
		for (int i = 0; i < sh.nbFacet; i++) {
			if (facets[i]->sh.superIdx == j || facets[i]->sh.superIdx == -1)
				DrawFacet(facets[i], false, true, false);
		}
		glEndList();
	}
    BuildAABBList();
	BuildVolumeFacetList();
	BuildNonPlanarList();
	BuildSelectList();

}

int Geometry::InvalidateDeviceObjects() {

	DeleteGLLists(true, true);
	DELETE_LIST(arrowList);
	DELETE_LIST(sphereList);
	for (int i = 0; i < sh.nbFacet; i++)
		facets[i]->InvalidateDeviceObjects();

	return GL_OK;

}

int Geometry::RestoreDeviceObjects() {

	if (!IsLoaded()) return GL_OK;

	for (int i = 0; i < sh.nbFacet; i++) {
		InterfaceFacet *f = facets[i];
		f->RestoreDeviceObjects();
		BuildFacetList(f);
	}

	BuildGLList();

	return GL_OK;

}

void Geometry::BuildFacetList(InterfaceFacet *f) {

	// Rebuild OpenGL geometry with texture

	if (f->sh.isTextured) {

		// Facet geometry
		glNewList(f->glList, GL_COMPILE);
		if (f->sh.nbIndex == 3) {
			glBegin(GL_TRIANGLES);
			FillFacet(f, true);
			glEnd();
		}
		else if (f->sh.nbIndex == 4) {

			glBegin(GL_QUADS);
			FillFacet(f, true);
			glEnd();
		}
		else {

			glBegin(GL_TRIANGLES);
			Triangulate(f, true);
			glEnd();
		}
		glEndList();
	}
}