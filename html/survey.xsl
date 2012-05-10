<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">

	<html>

		<!-- HTML Head section that links in CSS and javascript -->
		<head>
			<title>Survey</title>
			<script type="text/javascript" src="survey.js"></script>
			<link rel="stylesheet" type="text/css" href="survey.css" media="all" />
		</head>
		
		<!-- The body of the survey layout -->
		<body onLoad="addInstructor();addInstructor();">
			<form name="survey" action="ACTIONSCRIPT" method="POST">
				<div class="sectionhdr">Course Material And Comments</div>
				<br />
				<div>
					<table>
						<xsl:apply-templates select="/survey/course/field" /> 
						<tr style="height: 10pt;" />
						<xsl:call-template name="show_rating_scale" />
						<xsl:apply-templates select="/survey/course/rating" /> 
						<xsl:apply-templates select="/survey/course/question" /> 
					</table>
				</div>
				<br />
				<div class="sectionhdr">Instructor Reviews</div>
				
				<div id="instructor-reviews"><!-- Javascript inserts instructor forms here --></div>
				
				<input type="button" value="Add Instructor" name="add-instructor" onClick="addInstructor();" />
				<br /><br />
				<div class="sectionhdr">Optional POC Information</div>
				<br />
				<div>
					Name: <input type="text" name="name" id="id-poc-name" />
					SID: <input type="text" name="sid" id="id-poc-sid" />
				</div>
				<br /><input type="submit" name="submit-survey" />
			</form>
		</body>

		<!-- Template of instructor form used by javascript -->
		<div id="instructor-review-template" style="display: none;">
			<p>        
			<table>
				<xsl:apply-templates select="/survey/instructor/field" /> 
				<xsl:call-template name="show_rating_scale" />
				<xsl:apply-templates select="/survey/instructor/rating" /> 
				<xsl:apply-templates select="/survey/instructor/question" /> 
			</table>
			<div class="instructorhdr"><!-- This is a visual break between instructor forms. --> </div>
			</p>
		</div>

	</html>
</xsl:template>

<xsl:template name="show_rating_scale">
	<tr>
		<td></td>
		<xsl:apply-templates select="/survey/rate-scale/rate" /> 
	</tr>
</xsl:template>

<xsl:template match="field">
	<tr class="rating">
		<td class="ratingdesc"><xsl:value-of select="." />:</td>
		<xsl:element name="td">
			<xsl:attribute name="colspan">
				<xsl:value-of select="count(/survey/rate-scale/rate)" />
			</xsl:attribute>
			<xsl:element name="input">
				<xsl:attribute name="type">text</xsl:attribute>
				<xsl:attribute name="name"><xsl:value-of select="@attr" /></xsl:attribute>
				<xsl:attribute name="value"><xsl:value-of select="@default" /></xsl:attribute>
				<xsl:attribute name="id">id-<xsl:value-of select="@attr" /><xsl:if test="name(..) = 'instructor'">-NUM</xsl:if>
				</xsl:attribute>
			</xsl:element>
		</xsl:element>
	</tr>
</xsl:template>

<xsl:template match="question">
	<tr class="rating">
		<td class="ratingdesc"><xsl:value-of select="." />:</td>
		<xsl:element name="td">
			<xsl:attribute name="colspan">
				<xsl:value-of select="count(/survey/rate-scale/rate)+1" />
			</xsl:attribute>
			<xsl:element name="textarea">
				<xsl:attribute name="name"><xsl:value-of select="@attr" /></xsl:attribute>
				<xsl:attribute name="id">id-<xsl:value-of select="@attr" /></xsl:attribute>
				<xsl:attribute name="cols">40</xsl:attribute>
				<xsl:attribute name="rows">8</xsl:attribute>
			</xsl:element>
		</xsl:element>
	</tr>
</xsl:template>

<xsl:template match="rating">
	<xsl:variable name="attr"><xsl:value-of select="@attr" /></xsl:variable>
	<tr class="rating">
		<td class="ratingdesc"><xsl:value-of select="." />:</td>
		<xsl:apply-templates select="/survey/rate-scale/rate">
			<xsl:with-param name="isopt">yes</xsl:with-param>
			<xsl:with-param name="attr"><xsl:value-of select="$attr" /></xsl:with-param>
		</xsl:apply-templates>
	</tr>
</xsl:template>

<xsl:template match="rate">
	<xsl:param name="attr" />
	<xsl:param name="isopt" />
	<xsl:choose>
		<xsl:when test="$isopt='yes'">
			<xsl:call-template name="add_rating_option">
				<xsl:with-param name="attr"><xsl:value-of select="$attr" /></xsl:with-param>
				<xsl:with-param name="rate"><xsl:value-of select="@value" /></xsl:with-param>
			</xsl:call-template>
		</xsl:when>
		<xsl:otherwise>
			<xsl:call-template name="add_rating_hdr">
				<xsl:with-param name="rate"><xsl:value-of select="." /></xsl:with-param>
			</xsl:call-template>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template name="add_rating_hdr">
	<xsl:param name="rate" />
	<td><xsl:value-of select="$rate" /></td>
</xsl:template>

<xsl:template name="add_rating_option">
	<xsl:param name="rate" />
	<xsl:param name="attr" />
	<td class="rating">
		<xsl:element name="input">
			<xsl:attribute name="type">radio</xsl:attribute>
			<xsl:attribute name="name"><xsl:value-of select="$attr" />NUM</xsl:attribute>
			<xsl:attribute name="value"><xsl:value-of select="$rate" /></xsl:attribute>
			<xsl:attribute name="id">id-<xsl:value-of select="$attr" />-<xsl:value-of select="$rate" />-NUM</xsl:attribute>
		</xsl:element>
	</td>
</xsl:template>
			
</xsl:stylesheet>
