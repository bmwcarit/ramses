//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "LogicViewerLuaTypes.h"
#include "UpdateReportSummary.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LogicEngine.h"
#include "Result.h"
#include <string>
#include <chrono>

namespace rlogic
{
    class LogicEngine;
    struct LogicViewerSettings;
    class Property;

    class LogicViewer
    {
    public:
        using ScreenshotFunc = std::function<bool(const std::string&)>;

        // global tokens
        static const char* const ltnModule;
        static const char* const ltnScript;
        static const char* const ltnInterface;
        static const char* const ltnAnimation;
        static const char* const ltnTimer;
        static const char* const ltnNode;
        static const char* const ltnAppearance;
        static const char* const ltnCamera;
        static const char* const ltnRenderPass;
        static const char* const ltnRenderGroup;
        static const char* const ltnMeshNode;
        static const char* const ltnAnchorPoint;
        static const char* const ltnSkinBinding;
        static const char* const ltnScreenshot;
        static const char* const ltnViews;
        static const char* const ltnLink;
        static const char* const ltnUnlink;
        static const char* const ltnUpdate;
        static const char* const ltnIN;
        static const char* const ltnOUT;
        // property type
        static const char* const ltnPropertyValue;
        // view type
        static const char* const ltnViewUpdate;
        static const char* const ltnViewInputs;
        static const char* const ltnViewName;
        static const char* const ltnViewDescription;

        class View
        {
        public:
            explicit View(sol::optional<sol::table>&& tbl)
                : m_tbl(std::move(tbl))
            {
            }

            [[nodiscard]] bool isValid() const
            {
                return m_tbl ? true : false;
            }

            [[nodiscard]] std::string name() const
            {
                if (m_tbl)
                    return (*m_tbl).get_or(ltnViewName, std::string());
                return std::string();
            }

            [[nodiscard]] size_t getInputCount() const
            {
                if (m_tbl)
                {
                    sol::optional<sol::table> inputs = (*m_tbl)[ltnViewInputs];
                    if (inputs)
                    {
                        return (*inputs).size();
                    }
                }
                return 0U;
            }

            [[nodiscard]] Property* getInput(size_t index) const
            {
                if (m_tbl)
                {
                    sol::optional<PropertyWrapper> input = (*m_tbl)[ltnViewInputs][index + 1];
                    if (input)
                    {
                        return &input->m_property;
                    }
                }
                return nullptr;
            }

            [[nodiscard]] std::string description() const
            {
                if (m_tbl)
                    return (*m_tbl).get_or(ltnViewDescription, std::string());
                return std::string();
            }

        private:
            sol::optional<sol::table> m_tbl;
        };

        LogicViewer(EFeatureLevel engineFeatureLevel, ScreenshotFunc screenshotFunc);

        [[nodiscard]] bool loadRamsesLogic(const std::string& filename, ramses::Scene* scene);

        [[nodiscard]] const std::string& getLogicFilename() const;

        [[nodiscard]] Result loadLuaFile(const std::string& filename);

        [[nodiscard]] Result exec(const std::string& lua);

        [[nodiscard]] Result call(const std::string& functionName);

        [[nodiscard]] const std::string& getLuaFilename() const;

        [[nodiscard]] rlogic::LogicEngine& getEngine();

        [[nodiscard]] Result update();

        [[nodiscard]] size_t getViewCount() const;

        void setCurrentView(size_t viewId);

        [[nodiscard]] size_t  getCurrentView() const;

        [[nodiscard]] View getView(size_t viewId) const;

        [[nodiscard]] const Result& getLastResult() const;

        void enableUpdateReport(bool enable, size_t interval);

        [[nodiscard]] bool isUpdateReportEnabled() const;

        [[nodiscard]] const UpdateReportSummary& getUpdateReport() const;

        /**
         * saves a simple default lua configuration that can be used as a starting point
         */
        [[nodiscard]] Result saveDefaultLuaFile(const std::string& filename, const LogicViewerSettings& settings);

    private:
        void updateEngine();

        void load(sol::load_result&& loadResult);

        rlogic::LogicEngine m_logicEngine;
        ScreenshotFunc      m_screenshotFunc;
        std::string         m_logicFilename;
        std::string         m_luaFilename;
        sol::state          m_sol;
        size_t              m_view = 1U;
        Result              m_result;

        std::chrono::steady_clock::time_point m_startTime;

        bool                m_updateReportEnabled = false;
        UpdateReportSummary m_updateReportSummary;
    };

    inline LogicEngine& LogicViewer::getEngine()
    {
        return m_logicEngine;
    }

    inline const std::string& LogicViewer::getLuaFilename() const
    {
        return m_luaFilename;
    }

    inline const std::string& LogicViewer::getLogicFilename() const
    {
        return m_logicFilename;
    }

    inline const Result& LogicViewer::getLastResult() const
    {
        return m_result;
    }

    inline void LogicViewer::enableUpdateReport(bool enable, size_t interval)
    {
        m_logicEngine.enableUpdateReport(enable);
        m_updateReportSummary.setInterval(interval);
        m_updateReportEnabled = enable;
    }

    inline bool LogicViewer::isUpdateReportEnabled() const
    {
        return m_updateReportEnabled;
    }

    inline const UpdateReportSummary& LogicViewer::getUpdateReport() const
    {
        return m_updateReportSummary;
    }

    inline size_t LogicViewer::getCurrentView() const
    {
        return m_view;
    }
}

