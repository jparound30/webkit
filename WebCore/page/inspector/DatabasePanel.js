/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

WebInspector.DatabasePanel = function(database, views)
{
    var allViews = [{ title: "Query" }, { title: "Browse" }];
    if (views)
        allViews = allViews.concat(views);

    WebInspector.ResourcePanel.call(this, database, allViews);

    this.currentView = this.views.browse;

    this.queryPromptElement = document.createElement("textarea");
    this.queryPromptElement.className = "database-prompt";
    this.element.appendChild(this.queryPromptElement);

    var panel = this;
    this.queryPromptElement.addEventListener("keydown", function(event) { panel.queryInputKeypress(event) }, false);

    this.queryPromptHistory = [];
    this.queryPromptHistoryOffset = 0;

    var queryView = this.views.query;

    queryView.commandListElement = document.createElement("ol");
    queryView.commandListElement.className = "database-command-list";
    queryView.contentElement.appendChild(queryView.commandListElement);

    queryView.show = function()
    {
        panel.queryPromptElement.focus();
        this.commandListElement.scrollTop = this.previousScrollTop;
    };

    queryView.hide = function()
    {
        this.previousScrollTop = this.commandListElement.scrollTop;
    };

    var browseView = this.views.browse;

    browseView.reloadTableElement = document.createElement("button");
    browseView.reloadTableElement.appendChild(document.createElement("img"));
    browseView.reloadTableElement.className = "database-table-reload";
    browseView.reloadTableElement.title = "Reload";
    browseView.reloadTableElement.addEventListener("click", function() { panel.updateTableList(); panel.updateTableBrowser() }, false);

    browseView.show = function()
    {
        panel.updateTableList();
        panel.queryPromptElement.focus();

        this.tableSelectElement.removeStyleClass("hidden");
        if (!this.tableSelectElement.parentNode)
            document.getElementById("toolbarButtons").appendChild(this.tableSelectElement);

        this.reloadTableElement.removeStyleClass("hidden");
        if (!this.reloadTableElement.parentNode)
            document.getElementById("toolbarButtons").appendChild(this.reloadTableElement);

        this.contentElement.scrollTop = this.previousScrollTop;
    };

    browseView.hide = function()
    {
        this.tableSelectElement.addStyleClass("hidden");
        this.reloadTableElement.addStyleClass("hidden");
        this.previousScrollTop = this.contentElement.scrollTop;
    };
}

// FIXME: The function and local variables are a workaround for http://bugs.webkit.org/show_bug.cgi?id=15574.
WebInspector.DatabasePanel.prototype = (function() {
var document = window.document;
var Math = window.Math;
return {
    show: function()
    {
        WebInspector.ResourcePanel.prototype.show.call(this);
        this.queryPromptElement.focus();
    },

    get currentTable()
    {
        return this._currentTable;
    },

    set currentTable(x)
    {
        if (this._currentTable === x)
            return;

        this._currentTable = x;

        if (x) {
            var browseView = this.views.browse;
            if (browseView.tableSelectElement) {
                var length = browseView.tableSelectElement.options.length;
                for (var i = 0; i < length; ++i) {
                    var option = browseView.tableSelectElement.options[i];
                    if (option.value === x) {
                        browseView.tableSelectElement.selectedIndex = i;
                        break;
                    }
                }
            }

            this.updateTableBrowser();
        }
    },

    updateTableList: function()
    {
        var browseView = this.views.browse;
        if (!browseView.tableSelectElement) {
            browseView.tableSelectElement = document.createElement("select");
            browseView.tableSelectElement.className = "database-table-select hidden";

            var panel = this;
            var changeTableFunction = function()
            {
                var browseView = panel.views.browse;
                var index = browseView.tableSelectElement.selectedIndex;
                if (index != -1)
                    panel.currentTable = browseView.tableSelectElement.options[index].value;
                else
                    panel.currentTable = null;
            };

            browseView.tableSelectElement.addEventListener("change", changeTableFunction, false);
        }

        browseView.tableSelectElement.removeChildren();

        var selectedTableName = this.currentTable;
        var tableNames = InspectorController.databaseTableNames(this.resource.database).sort();

        var length = tableNames.length;
        for (var i = 0; i < length; ++i) {
            var option = document.createElement("option");
            option.value = tableNames[i];
            option.text = tableNames[i];
            browseView.tableSelectElement.appendChild(option);

            if (tableNames[i] === selectedTableName)
                browseView.tableSelectElement.selectedIndex = i;
        }

        if (!selectedTableName && length)
            this.currentTable = tableNames[0];
    },

    updateTableBrowser: function()
    {
        if (!this.currentTable) {
            this.views.browse.contentElement.removeChildren();
            return;
        }

        try {
            var panel = this;
            var query = "SELECT * FROM " + this.currentTable;
            this.resource.database.executeSql(query, [], function(result) { panel.browseQueryFinished(result) });
        } catch(e) {
            // FIXME: handle this error a better way.
            this.views.browse.contentElement.removeChildren();
        }
    },

    browseQueryFinished: function(result)
    {
        this.views.browse.contentElement.removeChildren();

        var table = this._tableForResult(result);
        if (table) {
            var rowCount = table.getElementsByTagName("tr").length;
            var columnCount = table.getElementsByTagName("tr").item(0).getElementsByTagName("th").length;

            var tr = document.createElement("tr");
            tr.className = "database-result-filler-row";
            table.appendChild(tr);

            if (!(rowCount % 2))
                tr.addStyleClass("alternate");

            for (var i = 0; i < columnCount; ++i) {
                var td = document.createElement("td");
                tr.appendChild(td);
            }

            table.addStyleClass("database-browse-table");
            this.views.browse.contentElement.appendChild(table);
        } else {
            if (!result.errorCode) {
                var emptyMsgElement = document.createElement("div");
                emptyMsgElement.className = "database-table-empty";
                emptyMsgElement.innerHTML = "The <q>" + this.currentTable + "</q><br>table is empty.";
                this.views.browse.contentElement.appendChild(emptyMsgElement);
            } else {
                var errorMsgElement = document.createElement("div");
                errorMsgElement.className = "database-table-error";
                errorMsgElement.innerHTML = "An error occurred trying to<br>read the <q>" + this.currentTable + "</q> table.";
                this.views.browse.contentElement.appendChild(errorMsgElement);
            }
        }
    },

    queryInputKeypress: function(event)
    {
        switch (event.keyIdentifier) {
            case "Enter":
                this._onQueryInputEnterPressed(event);
                break;
            case "Up":
                this._onQueryInputUpPressed(event);
                break;
            case "Down":
                this._onQueryInputDownPressed(event);
                break;
        }
    },

    queryFinished: function(query, result)
    {
        var commandItem = document.createElement("li");
        commandItem.className = "database-command";

        var queryDiv = document.createElement("div");
        queryDiv.className = "database-command-query";
        queryDiv.textContent = query;
        commandItem.appendChild(queryDiv);

        var resultDiv = document.createElement("div");
        resultDiv.className = "database-command-result";
        commandItem.appendChild(resultDiv);

        if (!result.errorCode) {
            var table = this._tableForResult(result);
            if (table)
                resultDiv.appendChild(table);
        } else {
            if (this.currentView !== this.views.query)
                this.currentView = this.views.query;

            if (result.errorCode == 1) {
                resultDiv.className += " error";
                resultDiv.textContent = result.error;
            } else if (result.errorCode == 2) {
                resultDiv.className += " error";
                resultDiv.textContent = "Database no longer has expected version.";
            } else {
                resultDiv.className += " error";
                resultDiv.textContent = "An unexpected error " + result.errorCode + " occured.";
            }
        }

        this.views.query.commandListElement.appendChild(commandItem);
        commandItem.scrollIntoView(false);
    },

    _onQueryInputEnterPressed: function(event)
    {
        event.preventDefault();
        event.stopPropagation();

        var query = this.queryPromptElement.value;
        if (!query.length)
            return;

        try {
            var panel = this;
            this.resource.database.executeSql(query, [], function(result) { panel.queryFinished(query, result) });

            this.queryPromptHistory.push(query);
            this.queryPromptHistoryOffset = 0;

            this.queryPromptElement.value = "";

            if (query.match(/^select /i)) {
                if (this.currentView !== this.views.query)
                    this.currentView = this.views.query;
            } else {
                if (query.match(/^create /i) || query.match(/^drop table /i))
                    this.updateTableList();

                // FIXME: we should only call updateTableBrowser() is we know the current table was modified
                this.updateTableBrowser();
            }
        } catch(e) {
            // FIXME: handle this error some way.
        }
    },

    _onQueryInputUpPressed: function(event)
    {
        event.preventDefault();
        event.stopPropagation();

        if (this.queryPromptHistoryOffset == this.queryPromptHistory.length)
            return;

        if (this.queryPromptHistoryOffset == 0)
            this.tempSavedQuery = this.queryPromptElement.value;

        ++this.queryPromptHistoryOffset;
        this.queryPromptElement.value = this.queryPromptHistory[this.queryPromptHistory.length - this.queryPromptHistoryOffset];
        this.queryPromptElement.moveCursorToEnd();
    },

    _onQueryInputDownPressed: function(event)
    {
        event.preventDefault();
        event.stopPropagation();

        if (this.queryPromptHistoryOffset == 0)
            return;

        --this.queryPromptHistoryOffset;

        if (this.queryPromptHistoryOffset == 0) {
            this.queryPromptElement.value = this.tempSavedQuery;
            this.queryPromptElement.moveCursorToEnd();
            delete this.tempSavedQuery;
            return;
        }

        this.queryPromptElement.value = this.queryPromptHistory[this.queryPromptHistory.length - this.queryPromptHistoryOffset];
        this.queryPromptElement.moveCursorToEnd();
    },

    _tableForResult: function(result)
    {
        if (result.errorCode || !result.rows.length)
            return null;

        var rows = result.rows;
        var length = rows.length;
        var columnWidths = [];

        var table = document.createElement("table");
        table.className = "database-result-table";

        var headerRow = document.createElement("tr");
        table.appendChild(headerRow);

        var j = 0;
        for (var column in rows.item(0)) {
            var th = document.createElement("th");
            headerRow.appendChild(th);

            var div = document.createElement("div");
            div.textContent = column;
            div.title = column;
            th.appendChild(div);

            columnWidths[j++] = column.length;
        }

        for (var i = 0; i < length; ++i) {
            var row = rows.item(i);
            var tr = document.createElement("tr");
            if (i % 2)
                tr.className = "alternate";
            table.appendChild(tr);

            var j = 0;
            for (var column in row) {
                var td = document.createElement("td");
                tr.appendChild(td);

                var text = row[column];
                var div = document.createElement("div");
                div.textContent = text;
                div.title = text;
                td.appendChild(div);

                if (text.length > columnWidths[j])
                    columnWidths[j] = text.length;
                ++j;
            }
        }

        var totalColumnWidths = 0;
        length = columnWidths.length;
        for (var i = 0; i < length; ++i)
            totalColumnWidths += columnWidths[i];

        // Calculate the percentage width for the columns.
        var minimumPrecent = 5;
        var recoupPercent = 0;
        for (var i = 0; i < length; ++i) {
            columnWidths[i] = Math.round((columnWidths[i] / totalColumnWidths) * 100);
            if (columnWidths[i] < minimumPrecent) {
                recoupPercent += (minimumPrecent - columnWidths[i]);
                columnWidths[i] = minimumPrecent;
            }
        }

        // Enforce the minimum percentage width.
        while (recoupPercent > 0) {
            for (var i = 0; i < length; ++i) {
                if (columnWidths[i] > minimumPrecent) {
                    --columnWidths[i];
                    --recoupPercent;
                    if (!recoupPercent)
                        break;
                }
            }
        }

        length = headerRow.childNodes.length;
        for (var i = 0; i < length; ++i) {
            var th = headerRow.childNodes[i];
            th.style.width = columnWidths[i] + "%";
        }

        return table;
    }
}
})();

WebInspector.DatabasePanel.prototype.__proto__ = WebInspector.ResourcePanel.prototype;
